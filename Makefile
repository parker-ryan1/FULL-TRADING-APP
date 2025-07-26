CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread
INCLUDES = -Iinclude
LIBS = -lpthread

# Directories
SRCDIR = src
BUILDDIR = build
BINDIR = bin
SERVICEDIR = services

# Services
SERVICES = orderbook options algo-trading backtesting risk market-data

.PHONY: all clean build-all $(SERVICES)

all: build-all

build-all: $(SERVICES)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

orderbook: $(BUILDDIR) $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BINDIR)/orderbook \
		$(SERVICEDIR)/orderbook/main.cpp \
		$(SERVICEDIR)/orderbook/orderbook.cpp \
		$(SERVICEDIR)/orderbook/order.cpp \
		$(SRCDIR)/common/database.cpp \
		$(SRCDIR)/common/messaging.cpp \
		$(LIBS)

options: $(BUILDDIR) $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BINDIR)/options \
		$(SERVICEDIR)/options/main.cpp \
		$(SERVICEDIR)/options/black_scholes.cpp \
		$(SERVICEDIR)/options/brownian_motion.cpp \
		$(SRCDIR)/common/database.cpp \
		$(SRCDIR)/common/messaging.cpp \
		$(LIBS)

algo-trading: $(BUILDDIR) $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BINDIR)/algo-trading \
		$(SERVICEDIR)/algo-trading/main.cpp \
		$(SERVICEDIR)/algo-trading/algo_engine.cpp \
		$(SERVICEDIR)/algo-trading/momentum_strategy.cpp \
		$(SERVICEDIR)/algo-trading/options_strategy.cpp \
		$(SERVICEDIR)/options/black_scholes.cpp \
		$(SERVICEDIR)/options/brownian_motion.cpp \
		$(SRCDIR)/common/database.cpp \
		$(SRCDIR)/common/messaging.cpp \
		$(LIBS)

lstm: $(BUILDDIR) $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BINDIR)/lstm \
		$(SERVICEDIR)/lstm/main.cpp \
		$(SERVICEDIR)/lstm/lstm_model.cpp \
		$(SERVICEDIR)/lstm/data_processor.cpp \
		$(SRCDIR)/common/database.cpp \
		$(SRCDIR)/common/messaging.cpp \
		$(LIBS)

backtesting: $(BUILDDIR) $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BINDIR)/backtesting \
		$(SERVICEDIR)/backtesting/main.cpp \
		$(SERVICEDIR)/backtesting/backtesting_engine.cpp \
		$(SERVICEDIR)/algo-trading/momentum_strategy.cpp \
		$(SERVICEDIR)/algo-trading/options_strategy.cpp \
		$(SERVICEDIR)/options/black_scholes.cpp \
		$(SERVICEDIR)/options/brownian_motion.cpp \
		$(SRCDIR)/common/database.cpp \
		$(SRCDIR)/common/messaging.cpp \
		$(LIBS)

risk: $(BUILDDIR) $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BINDIR)/risk \
		$(SERVICEDIR)/risk/main.cpp \
		$(SERVICEDIR)/risk/risk_manager.cpp \
		$(SRCDIR)/common/database.cpp \
		$(SRCDIR)/common/messaging.cpp \
		$(LIBS)

market-data: $(BUILDDIR) $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BINDIR)/market-data \
		$(SERVICEDIR)/market-data/main.cpp \
		$(SERVICEDIR)/market-data/polygon_client.cpp \
		$(SRCDIR)/common/database.cpp \
		$(SRCDIR)/common/messaging.cpp \
		$(LIBS)

clean:
	rm -rf $(BUILDDIR) $(BINDIR)

install-deps:
	# Install required dependencies
	sudo apt-get update
	sudo apt-get install -y build-essential cmake libssl-dev libpq-dev libhiredis-dev

docker-up:
	docker-compose up -d

docker-down:
	docker-compose down

test:
	cd tests && make test