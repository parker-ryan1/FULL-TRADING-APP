#!/bin/bash

echo "🚀 Starting Hedge Fund Trading Platform Services..."

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Start infrastructure if not running
print_status "Ensuring infrastructure services are running..."
docker-compose up -d

# Wait for services to be ready
print_status "Waiting for services to initialize..."
sleep 5

# Start C++ services in background
print_status "Starting Order Book Service..."
if [ -f "./bin/orderbook" ]; then
    ./bin/orderbook &
    ORDERBOOK_PID=$!
    echo $ORDERBOOK_PID > /tmp/orderbook.pid
else
    print_warning "Order Book service not found. Run 'make build-all' first."
fi

print_status "Starting Options Pricing Service..."
if [ -f "./bin/options" ]; then
    ./bin/options &
    OPTIONS_PID=$!
    echo $OPTIONS_PID > /tmp/options.pid
else
    print_warning "Options service not found. Run 'make build-all' first."
fi

print_status "Starting Risk Management Service..."
if [ -f "./bin/risk" ]; then
    ./bin/risk &
    RISK_PID=$!
    echo $RISK_PID > /tmp/risk.pid
else
    print_warning "Risk service not found. Run 'make build-all' first."
fi

# Start frontend development server
print_status "Starting React frontend..."
cd frontend
if [ -d "node_modules" ]; then
    npm start &
    FRONTEND_PID=$!
    echo $FRONTEND_PID > /tmp/frontend.pid
    cd ..
else
    print_warning "Frontend dependencies not installed. Run 'npm install' in frontend directory."
    cd ..
fi

print_status "✅ Services started!"
echo ""
echo "🌐 Access Points:"
echo "  Frontend:        http://localhost:3000"
echo "  Grafana:         http://localhost:3000 (admin/admin)"
echo "  TurboMQ Console: http://localhost:8161 (admin/admin)"
echo "  TimescaleDB:     localhost:5432 (trader/secure_password)"
echo ""
echo "🛑 To stop all services, run: ./scripts/stop-services.sh"
echo ""
echo "📊 Service Status:"
docker-compose ps

# Keep script running to show logs
print_status "Services are running. Press Ctrl+C to view stop instructions."
trap 'echo ""; echo "To stop services, run: ./scripts/stop-services.sh"; exit 0' INT
while true; do
    sleep 1
done