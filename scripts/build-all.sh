#!/bin/bash

echo "🚀 Building Hedge Fund Trading Platform..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if Docker is running
if ! docker info > /dev/null 2>&1; then
    print_error "Docker is not running. Please start Docker first."
    exit 1
fi

# Check if Node.js is installed
if ! command -v node &> /dev/null; then
    print_error "Node.js is not installed. Please install Node.js first."
    exit 1
fi

# Check if make is installed
if ! command -v make &> /dev/null; then
    print_error "Make is not installed. Please install build tools first."
    exit 1
fi

print_status "Starting infrastructure services..."
docker-compose up -d

print_status "Waiting for services to be ready..."
sleep 10

print_status "Installing frontend dependencies..."
cd frontend
npm install
if [ $? -ne 0 ]; then
    print_error "Failed to install frontend dependencies"
    exit 1
fi

print_status "Building React frontend..."
npm run build
if [ $? -ne 0 ]; then
    print_error "Failed to build frontend"
    exit 1
fi

cd ..

print_status "Installing system dependencies for C++ services..."
# For Ubuntu/Debian
if command -v apt-get &> /dev/null; then
    sudo apt-get update
    sudo apt-get install -y build-essential cmake libssl-dev libpq-dev libhiredis-dev pkg-config
fi

# For macOS
if command -v brew &> /dev/null; then
    brew install postgresql libpq hiredis openssl cmake
fi

print_status "Building C++ services..."
make clean
make build-all

if [ $? -ne 0 ]; then
    print_error "Failed to build C++ services"
    print_warning "You may need to install additional dependencies:"
    print_warning "  - PostgreSQL development libraries"
    print_warning "  - Redis/Hiredis libraries"
    print_warning "  - OpenSSL development libraries"
    exit 1
fi

print_status "Setting up database..."
# Wait for PostgreSQL to be ready
until docker-compose exec timescaledb pg_isready -U trader; do
    print_status "Waiting for PostgreSQL to be ready..."
    sleep 2
done

print_status "Database is ready!"

print_status "✅ Build completed successfully!"
echo ""
echo "🎯 Next steps:"
echo "1. Start the services: ./scripts/start-services.sh"
echo "2. Access the frontend: http://localhost:3000"
echo "3. Access Grafana: http://localhost:3000 (admin/admin)"
echo "4. Access TurboMQ console: http://localhost:8161 (admin/admin)"
echo ""
echo "📊 Service Status:"
docker-compose ps