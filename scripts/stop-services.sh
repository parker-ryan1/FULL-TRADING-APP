#!/bin/bash

echo "🛑 Stopping Hedge Fund Trading Platform Services..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Stop C++ services
print_status "Stopping C++ services..."

if [ -f "/tmp/orderbook.pid" ]; then
    PID=$(cat /tmp/orderbook.pid)
    if kill -0 $PID 2>/dev/null; then
        kill $PID
        print_status "Stopped Order Book service (PID: $PID)"
    fi
    rm -f /tmp/orderbook.pid
fi

if [ -f "/tmp/options.pid" ]; then
    PID=$(cat /tmp/options.pid)
    if kill -0 $PID 2>/dev/null; then
        kill $PID
        print_status "Stopped Options service (PID: $PID)"
    fi
    rm -f /tmp/options.pid
fi

if [ -f "/tmp/risk.pid" ]; then
    PID=$(cat /tmp/risk.pid)
    if kill -0 $PID 2>/dev/null; then
        kill $PID
        print_status "Stopped Risk Management service (PID: $PID)"
    fi
    rm -f /tmp/risk.pid
fi

# Stop frontend
if [ -f "/tmp/frontend.pid" ]; then
    PID=$(cat /tmp/frontend.pid)
    if kill -0 $PID 2>/dev/null; then
        kill $PID
        print_status "Stopped Frontend service (PID: $PID)"
    fi
    rm -f /tmp/frontend.pid
fi

# Stop Docker services
print_status "Stopping Docker services..."
docker-compose down

print_status "✅ All services stopped!"