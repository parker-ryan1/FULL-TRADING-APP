import React, { useState, useEffect } from 'react';
import styled from 'styled-components';

const AlgoContainer = styled.div`
  padding: 20px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border-radius: 15px;
  color: white;
  margin: 20px 0;
`;

const AlgoHeader = styled.h2`
  margin-bottom: 30px;
  text-align: center;
  font-size: 2.2em;
  text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
`;

const StrategyGrid = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
  gap: 20px;
  margin-bottom: 30px;
`;

const StrategyCard = styled.div`
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-radius: 12px;
  padding: 20px;
  border: 1px solid rgba(255, 255, 255, 0.2);
  transition: all 0.3s ease;

  &:hover {
    transform: translateY(-5px);
    box-shadow: 0 10px 25px rgba(0,0,0,0.2);
  }
`;

const StrategyTitle = styled.h3`
  margin-bottom: 15px;
  color: #fff;
  display: flex;
  align-items: center;
  justify-content: space-between;
`;

const StatusBadge = styled.span`
  padding: 4px 12px;
  border-radius: 20px;
  font-size: 0.8em;
  font-weight: bold;
  background: ${props => props.active ? '#4CAF50' : '#f44336'};
  color: white;
`;

const StrategyMetrics = styled.div`
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 15px;
  margin: 15px 0;
`;

const Metric = styled.div`
  text-align: center;
  
  .label {
    font-size: 0.9em;
    opacity: 0.8;
    margin-bottom: 5px;
  }
  
  .value {
    font-size: 1.4em;
    font-weight: bold;
    color: ${props => props.positive ? '#4CAF50' : props.negative ? '#f44336' : '#fff'};
  }
`;

const ControlPanel = styled.div`
  background: rgba(255, 255, 255, 0.1);
  border-radius: 12px;
  padding: 20px;
  margin-top: 20px;
`;

const ControlGrid = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 15px;
  margin-bottom: 20px;
`;

const ControlButton = styled.button`
  padding: 12px 20px;
  border: none;
  border-radius: 8px;
  background: ${props => props.danger ? '#f44336' : props.success ? '#4CAF50' : '#2196F3'};
  color: white;
  font-weight: bold;
  cursor: pointer;
  transition: all 0.3s ease;

  &:hover {
    transform: translateY(-2px);
    box-shadow: 0 5px 15px rgba(0,0,0,0.2);
  }

  &:disabled {
    opacity: 0.6;
    cursor: not-allowed;
    transform: none;
  }
`;

const SignalsPanel = styled.div`
  background: rgba(255, 255, 255, 0.1);
  border-radius: 12px;
  padding: 20px;
  margin-top: 20px;
  max-height: 400px;
  overflow-y: auto;
`;

const SignalItem = styled.div`
  background: rgba(255, 255, 255, 0.05);
  border-radius: 8px;
  padding: 15px;
  margin-bottom: 10px;
  border-left: 4px solid ${props => 
    props.type === 'BUY' ? '#4CAF50' : 
    props.type === 'SELL' ? '#f44336' : 
    props.type === 'BUY_CALL' ? '#2196F3' :
    props.type === 'SELL_CALL' ? '#FF9800' :
    props.type === 'BUY_PUT' ? '#9C27B0' :
    props.type === 'SELL_PUT' ? '#E91E63' : '#666'
  };
  
  .signal-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 8px;
  }
  
  .signal-type {
    font-weight: bold;
    color: ${props => 
      props.type === 'BUY' ? '#4CAF50' : 
      props.type === 'SELL' ? '#f44336' : 
      '#2196F3'
    };
  }
  
  .signal-confidence {
    font-size: 0.9em;
    opacity: 0.8;
  }
  
  .signal-details {
    font-size: 0.9em;
    opacity: 0.9;
    line-height: 1.4;
  }
`;

const AlgorithmicTrading = () => {
  const [strategies, setStrategies] = useState([
    {
      id: 'momentum_1',
      name: 'Momentum Strategy',
      type: 'MOMENTUM',
      active: true,
      pnl: 2450.75,
      winRate: 68.5,
      sharpeRatio: 1.42,
      maxDrawdown: -3.2,
      positions: 5,
      signals: 23
    },
    {
      id: 'options_straddle_1',
      name: 'Options Straddle',
      type: 'OPTIONS_STRADDLE',
      active: true,
      pnl: 1875.30,
      winRate: 72.1,
      sharpeRatio: 1.68,
      maxDrawdown: -2.8,
      positions: 3,
      signals: 12
    },
    {
      id: 'iron_condor_1',
      name: 'Iron Condor',
      type: 'IRON_CONDOR',
      active: false,
      pnl: -125.50,
      winRate: 45.2,
      sharpeRatio: 0.85,
      maxDrawdown: -5.1,
      positions: 2,
      signals: 8
    },
    {
      id: 'covered_call_1',
      name: 'Covered Call',
      type: 'COVERED_CALL',
      active: true,
      pnl: 3200.15,
      winRate: 85.7,
      sharpeRatio: 2.15,
      maxDrawdown: -1.5,
      positions: 8,
      signals: 31
    }
  ]);

  const [recentSignals, setRecentSignals] = useState([
    {
      id: 1,
      strategy: 'Momentum Strategy',
      symbol: 'AAPL',
      type: 'BUY',
      price: 150.25,
      confidence: 0.85,
      reason: 'Strong upward momentum detected',
      timestamp: new Date(Date.now() - 300000)
    },
    {
      id: 2,
      strategy: 'Options Straddle',
      symbol: 'GOOGL',
      type: 'BUY_CALL',
      price: 8.50,
      strike: 2800,
      confidence: 0.75,
      reason: 'Low volatility - expecting breakout',
      timestamp: new Date(Date.now() - 600000)
    },
    {
      id: 3,
      strategy: 'Options Straddle',
      symbol: 'GOOGL',
      type: 'BUY_PUT',
      price: 9.25,
      strike: 2800,
      confidence: 0.75,
      reason: 'Long straddle setup complete',
      timestamp: new Date(Date.now() - 610000)
    },
    {
      id: 4,
      strategy: 'Covered Call',
      symbol: 'TSLA',
      type: 'SELL_CALL',
      price: 12.75,
      strike: 820,
      confidence: 0.80,
      reason: 'Generate income from stock position',
      timestamp: new Date(Date.now() - 900000)
    }
  ]);

  const [systemStatus, setSystemStatus] = useState({
    running: true,
    totalPnl: 7400.70,
    activeStrategies: 3,
    totalPositions: 18,
    riskLevel: 'MODERATE'
  });

  useEffect(() => {
    // Simulate real-time updates
    const interval = setInterval(() => {
      // Update strategy metrics
      setStrategies(prev => prev.map(strategy => ({
        ...strategy,
        pnl: strategy.pnl + (Math.random() - 0.5) * 50,
        signals: strategy.signals + (Math.random() > 0.8 ? 1 : 0)
      })));

      // Add new signals occasionally
      if (Math.random() > 0.7) {
        const newSignal = {
          id: Date.now(),
          strategy: 'Momentum Strategy',
          symbol: ['AAPL', 'GOOGL', 'TSLA'][Math.floor(Math.random() * 3)],
          type: ['BUY', 'SELL', 'BUY_CALL', 'SELL_CALL', 'BUY_PUT', 'SELL_PUT'][Math.floor(Math.random() * 6)],
          price: 100 + Math.random() * 100,
          confidence: 0.6 + Math.random() * 0.3,
          reason: 'Algorithmic signal generated',
          timestamp: new Date()
        };

        setRecentSignals(prev => [newSignal, ...prev.slice(0, 9)]);
      }
    }, 3000);

    return () => clearInterval(interval);
  }, []);

  const toggleStrategy = (strategyId) => {
    setStrategies(prev => prev.map(strategy => 
      strategy.id === strategyId 
        ? { ...strategy, active: !strategy.active }
        : strategy
    ));
  };

  const stopAllStrategies = () => {
    setStrategies(prev => prev.map(strategy => ({ ...strategy, active: false })));
    setSystemStatus(prev => ({ ...prev, running: false }));
  };

  const startAllStrategies = () => {
    setStrategies(prev => prev.map(strategy => ({ ...strategy, active: true })));
    setSystemStatus(prev => ({ ...prev, running: true }));
  };

  const formatCurrency = (value) => {
    return new Intl.NumberFormat('en-US', {
      style: 'currency',
      currency: 'USD'
    }).format(value);
  };

  const formatTime = (date) => {
    return date.toLocaleTimeString();
  };

  return (
    <AlgoContainer>
      <AlgoHeader>🤖 Algorithmic Trading Engine</AlgoHeader>
      
      <ControlPanel>
        <h3>System Control</h3>
        <ControlGrid>
          <ControlButton success onClick={startAllStrategies} disabled={systemStatus.running}>
            ▶️ Start All Strategies
          </ControlButton>
          <ControlButton danger onClick={stopAllStrategies} disabled={!systemStatus.running}>
            ⏹️ Stop All Strategies
          </ControlButton>
          <div style={{ textAlign: 'center' }}>
            <div className="label">Total P&L</div>
            <div className="value" style={{ color: systemStatus.totalPnl > 0 ? '#4CAF50' : '#f44336', fontSize: '1.5em' }}>
              {formatCurrency(systemStatus.totalPnl)}
            </div>
          </div>
          <div style={{ textAlign: 'center' }}>
            <div className="label">Risk Level</div>
            <div className="value" style={{ color: systemStatus.riskLevel === 'LOW' ? '#4CAF50' : systemStatus.riskLevel === 'HIGH' ? '#f44336' : '#FF9800' }}>
              {systemStatus.riskLevel}
            </div>
          </div>
        </ControlGrid>
      </ControlPanel>

      <StrategyGrid>
        {strategies.map(strategy => (
          <StrategyCard key={strategy.id}>
            <StrategyTitle>
              {strategy.name}
              <StatusBadge active={strategy.active}>
                {strategy.active ? 'ACTIVE' : 'STOPPED'}
              </StatusBadge>
            </StrategyTitle>
            
            <StrategyMetrics>
              <Metric positive={strategy.pnl > 0} negative={strategy.pnl < 0}>
                <div className="label">P&L</div>
                <div className="value">{formatCurrency(strategy.pnl)}</div>
              </Metric>
              <Metric>
                <div className="label">Win Rate</div>
                <div className="value">{strategy.winRate.toFixed(1)}%</div>
              </Metric>
              <Metric>
                <div className="label">Sharpe Ratio</div>
                <div className="value">{strategy.sharpeRatio.toFixed(2)}</div>
              </Metric>
              <Metric negative={strategy.maxDrawdown < 0}>
                <div className="label">Max Drawdown</div>
                <div className="value">{strategy.maxDrawdown.toFixed(1)}%</div>
              </Metric>
            </StrategyMetrics>
            
            <div style={{ display: 'flex', justifyContent: 'space-between', marginTop: '15px' }}>
              <span>Positions: {strategy.positions}</span>
              <span>Signals: {strategy.signals}</span>
            </div>
            
            <ControlButton 
              style={{ width: '100%', marginTop: '15px' }}
              onClick={() => toggleStrategy(strategy.id)}
            >
              {strategy.active ? 'Stop Strategy' : 'Start Strategy'}
            </ControlButton>
          </StrategyCard>
        ))}
      </StrategyGrid>

      <SignalsPanel>
        <h3>Recent Trading Signals</h3>
        {recentSignals.map(signal => (
          <SignalItem key={signal.id} type={signal.type}>
            <div className="signal-header">
              <div>
                <span className="signal-type">{signal.type}</span> {signal.symbol}
                {signal.strike && ` $${signal.strike}`}
              </div>
              <div className="signal-confidence">
                {(signal.confidence * 100).toFixed(0)}% confidence
              </div>
            </div>
            <div className="signal-details">
              <div>Strategy: {signal.strategy}</div>
              <div>Price: ${signal.price.toFixed(2)} | Time: {formatTime(signal.timestamp)}</div>
              <div>Reason: {signal.reason}</div>
            </div>
          </SignalItem>
        ))}
      </SignalsPanel>
    </AlgoContainer>
  );
};

export default AlgorithmicTrading;