# Hedge Fund Trading Platform - Setup Guide

## 🏗️ Architecture Overview

This is a comprehensive hedge fund trading platform with:

### Core Services (C++)
- **Order Book Engine** - High-performance order matching with price-time priority
- **Options Pricing Service** - Black-Scholes and Monte Carlo pricing with Greeks
- **Risk Management Service** - User-configurable risk parameters and monitoring
- **LSTM Analysis Service** - Time series prediction (placeholder for ML integration)

### Infrastructure
- **TimescaleDB** - Time-series optimized PostgreSQL for market data
- **TurboMQ (ActiveMQ Artemis)** - Message queuing for service coordination
- **Redis** - Caching and session management
- **Grafana** - Analytics dashboards and visualization

### Frontend
- **React Application** - Modern trading interface with real-time updates
- **Payment Integration** - PayPal and Stripe support (configured)
- **Real-time Charts** - Market data visualization

## 🚀 Quick Start (Windows)

### Prerequisites

1. **Docker Desktop** - Download from https://www.docker.com/products/docker-desktop
2. **Node.js** - Download from https://nodejs.org (LTS version recommended)
3. **Visual Studio Build Tools** or **Visual Studio Community** for C++ compilation
4. **Git** - For version control

### Installation Steps

1. **Clone and Setup**
   ```cmd
   git clone <your-repo>
   cd hedge-fund-platform
   ```

2. **Start Infrastructure Services**
   ```cmd
   docker-compose up -d
   ```

3. **Install Frontend Dependencies**
   ```cmd
   cd frontend
   npm install
   cd ..
   ```

4. **Build C++ Services** (Windows)
   ```cmd
   # Install vcpkg for C++ dependencies (one-time setup)
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   
   # Install required packages
   .\vcpkg install libpq:x64-windows
   .\vcpkg install hiredis:x64-windows
   .\vcpkg install openssl:x64-windows
   
   cd ..
   
   # Build services
   make build-all
   ```

5. **Start All Services**
   ```cmd
   # Start C++ services (in separate terminals)
   .\bin\orderbook.exe
   .\bin\options.exe
   .\bin\risk.exe
   
   # Start frontend (in another terminal)
   cd frontend
   npm start
   ```

## 🌐 Access Points

- **Trading Interface**: http://localhost:3000
- **Grafana Dashboard**: http://localhost:3000 (admin/admin)
- **TurboMQ Console**: http://localhost:8161 (admin/admin)
- **Database**: localhost:5432 (trader/secure_password)

## 📊 Features Implemented

### ✅ Core Trading Engine
- Order book with price-time priority matching
- Market, limit, stop, and stop-limit orders
- Real-time trade execution and reporting
- Position tracking and portfolio management

### ✅ Options Pricing
- Black-Scholes analytical pricing
- Monte Carlo simulation pricing
- Greeks calculation (Delta, Gamma, Theta, Vega, Rho)
- Implied volatility calculation
- Brownian motion simulation

### ✅ Risk Management
- User-configurable risk parameters
- Real-time risk monitoring
- VaR (Value at Risk) calculation
- Position limits and concentration limits
- Automated risk alerts

### ✅ Frontend Interface
- Modern React-based trading interface
- Real-time market data display
- Options trading interface with Greeks
- Risk management dashboard
- Portfolio overview

### ✅ Infrastructure
- TimescaleDB for time-series data
- Message queuing with TurboMQ
- Docker containerization
- Grafana integration ready

## 🔧 Configuration

### Database Configuration
The system uses TimescaleDB (PostgreSQL extension) optimized for time-series data:
- Market data with automatic partitioning
- Continuous aggregates for OHLCV data
- Optimized indexes for fast queries

### Message Queue Configuration
TurboMQ (ActiveMQ Artemis) handles inter-service communication:
- Order routing and execution
- Market data distribution
- Risk alerts and notifications

### Risk Parameters (User Configurable)
- Maximum position size
- Daily loss limits
- Portfolio VaR thresholds
- Leverage limits
- Concentration limits
- Stop-loss percentages

## 🚧 Next Steps for Full Implementation

### LSTM Integration
```cpp
// Add TensorFlow C++ or PyTorch C++ integration
// services/lstm/lstm_model.cpp - Neural network implementation
// Time series prediction and pattern recognition
```

### Payment Integration
```javascript
// Complete Stripe and PayPal integration
// Account funding and withdrawal processing
// Transaction history and fee management
```

### Advanced Analytics
```sql
-- Grafana dashboards for:
-- Portfolio performance metrics
-- Risk analytics and stress testing
-- Market data visualization
-- Trading performance analysis
```

## 🛠️ Development

### Adding New Services
1. Create service directory in `services/`
2. Implement main.cpp with message queue integration
3. Add to Makefile build targets
4. Update docker-compose.yml if needed

### Database Schema Extensions
```sql
-- Add new tables in sql/init.sql
-- Create hypertables for time-series data
-- Add appropriate indexes and continuous aggregates
```

### Frontend Components
```javascript
// Add new React components in frontend/src/components/
// Integrate with WebSocket for real-time updates
// Style with styled-components
```

## 📈 Performance Considerations

- **Order Book**: In-memory data structures with O(log n) operations
- **Database**: TimescaleDB partitioning and compression
- **Message Queue**: Asynchronous processing with TurboMQ
- **Frontend**: React optimization with useMemo and useCallback

## 🔒 Security Features

- Database connection encryption
- Message queue authentication
- Input validation and sanitization
- Risk limit enforcement
- Audit logging for all trades

## 📚 Documentation

- API documentation in `/docs/api/`
- Database schema in `/docs/database/`
- Architecture diagrams in `/docs/architecture/`
- Deployment guides in `/docs/deployment/`

## 🤝 Contributing

1. Fork the repository
2. Create feature branch
3. Implement changes with tests
4. Submit pull request

## 📄 License

This project is licensed under the MIT License - see LICENSE file for details.

---

**Note**: This is a comprehensive trading platform foundation. For production use, additional security, compliance, and regulatory features would need to be implemented based on your specific requirements and jurisdiction.