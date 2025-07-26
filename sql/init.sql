-- Initialize TimescaleDB extension
CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

-- Market data table (time-series)
CREATE TABLE IF NOT EXISTS market_data (
    symbol TEXT NOT NULL,
    price DECIMAL(10,4) NOT NULL,
    volume DECIMAL(15,2) DEFAULT 0,
    timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    bid DECIMAL(10,4),
    ask DECIMAL(10,4),
    spread DECIMAL(10,4)
);

-- Convert to hypertable for time-series optimization
SELECT create_hypertable('market_data', 'timestamp', if_not_exists => TRUE);

-- Trades table
CREATE TABLE IF NOT EXISTS trades (
    symbol TEXT NOT NULL,
    price DECIMAL(10,4) NOT NULL,
    quantity DECIMAL(15,2) NOT NULL,
    side VARCHAR(4) NOT NULL CHECK (side IN ('BUY', 'SELL', 'MATCHED')),
    timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    order_id VARCHAR(50),
    client_id VARCHAR(50)
);

SELECT create_hypertable('trades', 'timestamp', if_not_exists => TRUE);

-- Orders table (regular table, not hypertable due to primary key requirements)
CREATE TABLE IF NOT EXISTS orders (
    id VARCHAR(50) PRIMARY KEY,
    symbol VARCHAR(10) NOT NULL,
    order_type VARCHAR(10) NOT NULL CHECK (order_type IN ('MARKET', 'LIMIT', 'STOP', 'STOP_LIMIT')),
    side VARCHAR(4) NOT NULL CHECK (side IN ('BUY', 'SELL')),
    price DECIMAL(10,4),
    quantity DECIMAL(15,2) NOT NULL,
    filled_quantity DECIMAL(15,2) DEFAULT 0,
    status VARCHAR(20) NOT NULL DEFAULT 'PENDING',
    timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    client_id VARCHAR(50) NOT NULL
);

-- Options data table
CREATE TABLE IF NOT EXISTS options_data (
    underlying_symbol TEXT NOT NULL,
    strike_price DECIMAL(10,4) NOT NULL,
    expiration_date DATE NOT NULL,
    option_type VARCHAR(4) NOT NULL CHECK (option_type IN ('CALL', 'PUT')),
    implied_volatility DECIMAL(8,6),
    theoretical_price DECIMAL(10,4),
    delta DECIMAL(8,6),
    gamma DECIMAL(8,6),
    theta DECIMAL(8,6),
    vega DECIMAL(8,6),
    rho DECIMAL(8,6),
    timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

SELECT create_hypertable('options_data', 'timestamp', if_not_exists => TRUE);

-- Portfolio positions table (regular table, not hypertable due to unique constraint requirements)
CREATE TABLE IF NOT EXISTS positions (
    id SERIAL PRIMARY KEY,
    client_id VARCHAR(50) NOT NULL,
    symbol VARCHAR(10) NOT NULL,
    quantity DECIMAL(15,2) NOT NULL,
    average_price DECIMAL(10,4) NOT NULL,
    market_value DECIMAL(15,2),
    unrealized_pnl DECIMAL(15,2),
    timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE(client_id, symbol)
);

-- Risk metrics table
CREATE TABLE IF NOT EXISTS risk_metrics (
    client_id VARCHAR(50) NOT NULL,
    metric_type VARCHAR(20) NOT NULL,
    metric_value DECIMAL(15,4) NOT NULL,
    threshold_value DECIMAL(15,4),
    status VARCHAR(10) NOT NULL DEFAULT 'OK',
    timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

SELECT create_hypertable('risk_metrics', 'timestamp', if_not_exists => TRUE);

-- LSTM predictions table
CREATE TABLE IF NOT EXISTS lstm_predictions (
    symbol VARCHAR(10) NOT NULL,
    prediction_horizon INTEGER NOT NULL, -- hours ahead
    predicted_price DECIMAL(10,4) NOT NULL,
    confidence_score DECIMAL(5,4),
    actual_price DECIMAL(10,4),
    model_version VARCHAR(20),
    timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

SELECT create_hypertable('lstm_predictions', 'timestamp', if_not_exists => TRUE);

-- Create indexes for better performance
CREATE INDEX IF NOT EXISTS idx_market_data_symbol_time ON market_data (symbol, timestamp DESC);
CREATE INDEX IF NOT EXISTS idx_trades_symbol_time ON trades (symbol, timestamp DESC);
CREATE INDEX IF NOT EXISTS idx_orders_client_status ON orders (client_id, status);
CREATE INDEX IF NOT EXISTS idx_positions_client ON positions (client_id);
CREATE INDEX IF NOT EXISTS idx_risk_metrics_client_time ON risk_metrics (client_id, timestamp DESC);
CREATE INDEX IF NOT EXISTS idx_lstm_symbol_time ON lstm_predictions (symbol, timestamp DESC);

-- Create continuous aggregates for analytics
CREATE MATERIALIZED VIEW IF NOT EXISTS market_data_1min
WITH (timescaledb.continuous) AS
SELECT 
    time_bucket('1 minute', timestamp) AS bucket,
    symbol,
    FIRST(price, timestamp) as open,
    MAX(price) as high,
    MIN(price) as low,
    LAST(price, timestamp) as close,
    SUM(volume) as volume,
    COUNT(*) as tick_count
FROM market_data
GROUP BY bucket, symbol;

CREATE MATERIALIZED VIEW IF NOT EXISTS market_data_1hour
WITH (timescaledb.continuous) AS
SELECT 
    time_bucket('1 hour', timestamp) AS bucket,
    symbol,
    FIRST(price, timestamp) as open,
    MAX(price) as high,
    MIN(price) as low,
    LAST(price, timestamp) as close,
    SUM(volume) as volume,
    COUNT(*) as tick_count
FROM market_data
GROUP BY bucket, symbol;

-- Add refresh policies
SELECT add_continuous_aggregate_policy('market_data_1min',
    start_offset => INTERVAL '1 hour',
    end_offset => INTERVAL '1 minute',
    schedule_interval => INTERVAL '1 minute');

SELECT add_continuous_aggregate_policy('market_data_1hour',
    start_offset => INTERVAL '1 day',
    end_offset => INTERVAL '1 hour',
    schedule_interval => INTERVAL '1 hour');

-- Insert some sample data
INSERT INTO market_data (symbol, price, volume) VALUES 
('AAPL', 150.25, 1000),
('AAPL', 150.30, 1500),
('AAPL', 150.20, 800),
('GOOGL', 2800.50, 500),
('GOOGL', 2801.00, 750),
('TSLA', 800.75, 2000);

-- Create user for the application
CREATE USER hedgefund_app WITH PASSWORD 'app_password';
GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA public TO hedgefund_app;
GRANT USAGE, SELECT ON ALL SEQUENCES IN SCHEMA public TO hedgefund_app;