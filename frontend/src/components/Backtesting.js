import React, { useState, useEffect } from 'react';
import styled from 'styled-components';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, BarChart, Bar, ScatterChart, Scatter } from 'recharts';

const BacktestContainer = styled.div`
  padding: 20px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border-radius: 15px;
  color: white;
  margin: 20px 0;
`;

const BacktestHeader = styled.h2`
  margin-bottom: 30px;
  text-align: center;
  font-size: 2.2em;
  text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
`;

const ConfigPanel = styled.div`
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-radius: 12px;
  padding: 25px;
  border: 1px solid rgba(255, 255, 255, 0.2);
  margin-bottom: 30px;
`;

const ConfigGrid = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
  gap: 20px;
  margin-bottom: 20px;
`;

const FormGroup = styled.div`
  margin-bottom: 15px;
  
  label {
    display: block;
    margin-bottom: 8px;
    font-weight: 500;
    font-size: 0.9em;
  }
  
  input, select {
    width: 100%;
    padding: 10px;
    border: 1px solid rgba(255, 255, 255, 0.3);
    border-radius: 6px;
    background: rgba(255, 255, 255, 0.1);
    color: white;
    font-size: 14px;
    
    &::placeholder {
      color: rgba(255, 255, 255, 0.6);
    }
    
    &:focus {
      outline: none;
      border-color: #4CAF50;
    }
  }
`;

const RunButton = styled.button`
  padding: 15px 30px;
  border: none;
  border-radius: 8px;
  background: #4CAF50;
  color: white;
  font-size: 16px;
  font-weight: bold;
  cursor: pointer;
  transition: all 0.3s ease;
  margin-right: 15px;

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

const ResultsGrid = styled.div`
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 20px;
  margin-bottom: 30px;
  
  @media (max-width: 768px) {
    grid-template-columns: 1fr;
  }
`;

const ResultCard = styled.div`
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-radius: 12px;
  padding: 20px;
  border: 1px solid rgba(255, 255, 255, 0.2);
  
  h3 {
    margin-bottom: 20px;
    text-align: center;
  }
`;

const MetricsGrid = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
  gap: 15px;
  margin-bottom: 20px;
`;

const MetricItem = styled.div`
  text-align: center;
  padding: 15px;
  background: rgba(255, 255, 255, 0.05);
  border-radius: 8px;
  
  .metric-label {
    font-size: 0.8em;
    opacity: 0.8;
    margin-bottom: 8px;
  }
  
  .metric-value {
    font-size: 1.4em;
    font-weight: bold;
    color: ${props => props.positive ? '#4CAF50' : props.negative ? '#f44336' : '#fff'};
  }
`;

const ComparisonTable = styled.table`
  width: 100%;
  border-collapse: collapse;
  margin-top: 15px;
  
  th, td {
    padding: 10px;
    text-align: left;
    border-bottom: 1px solid rgba(255, 255, 255, 0.1);
    font-size: 0.9em;
  }
  
  th {
    background: rgba(255, 255, 255, 0.1);
    font-weight: bold;
  }
  
  tr:hover {
    background: rgba(255, 255, 255, 0.05);
  }
`;

const StatusBadge = styled.span`
  padding: 4px 8px;
  border-radius: 12px;
  font-size: 0.8em;
  font-weight: bold;
  background: ${props => 
    props.status === 'COMPLETED' ? '#4CAF50' : 
    props.status === 'RUNNING' ? '#FF9800' : 
    props.status === 'FAILED' ? '#f44336' : '#666'
  };
  color: white;
`;

const Backtesting = () => {
  const [config, setConfig] = useState({
    strategy: 'momentum',
    startDate: '2023-01-01',
    endDate: '2024-01-01',
    initialCapital: 1000000,
    symbols: 'AAPL,GOOGL,TSLA',
    commission: 5.0,
    slippage: 0.05
  });

  const [isRunning, setIsRunning] = useState(false);
  const [results, setResults] = useState(null);
  
  const [backtestHistory, setBacktestHistory] = useState([
    {
      id: 1,
      strategy: 'Momentum Strategy',
      period: '2023-01-01 to 2024-01-01',
      return: 24.7,
      sharpe: 1.42,
      maxDD: -3.1,
      trades: 156,
      status: 'COMPLETED',
      date: '2024-01-15'
    },
    {
      id: 2,
      strategy: 'Options Straddle',
      period: '2023-06-01 to 2024-01-01',
      return: 18.9,
      sharpe: 1.68,
      maxDD: -2.8,
      trades: 89,
      status: 'COMPLETED',
      date: '2024-01-14'
    },
    {
      id: 3,
      strategy: 'Iron Condor',
      period: '2023-01-01 to 2024-01-01',
      return: 12.4,
      sharpe: 1.15,
      maxDD: -1.9,
      trades: 234,
      status: 'COMPLETED',
      date: '2024-01-13'
    }
  ]);

  const [equityCurve, setEquityCurve] = useState([
    { date: '2023-01', value: 1000000, benchmark: 1000000, drawdown: 0 },
    { date: '2023-02', value: 1025000, benchmark: 1015000, drawdown: 0 },
    { date: '2023-03', value: 1018000, benchmark: 1008000, drawdown: -0.7 },
    { date: '2023-04', value: 1045000, benchmark: 1025000, drawdown: 0 },
    { date: '2023-05', value: 1038000, benchmark: 1020000, drawdown: -0.7 },
    { date: '2023-06', value: 1067000, benchmark: 1035000, drawdown: 0 },
    { date: '2023-07', value: 1089000, benchmark: 1048000, drawdown: 0 },
    { date: '2023-08', value: 1076000, benchmark: 1041000, drawdown: -1.2 },
    { date: '2023-09', value: 1095000, benchmark: 1055000, drawdown: 0 },
    { date: '2023-10', value: 1112000, benchmark: 1068000, drawdown: 0 },
    { date: '2023-11', value: 1134000, benchmark: 1082000, drawdown: 0 },
    { date: '2023-12', value: 1247000, benchmark: 1095000, drawdown: 0 }
  ]);

  const [monthlyReturns, setMonthlyReturns] = useState([
    { month: 'Jan', return: 2.5, benchmark: 1.5 },
    { month: 'Feb', return: -0.7, benchmark: -0.7 },
    { month: 'Mar', return: 2.6, benchmark: 1.7 },
    { month: 'Apr', return: -0.7, benchmark: -0.5 },
    { month: 'May', return: 2.8, benchmark: 1.5 },
    { month: 'Jun', return: 2.1, benchmark: 1.3 },
    { month: 'Jul', return: -1.2, benchmark: -0.7 },
    { month: 'Aug', return: 1.8, benchmark: 1.3 },
    { month: 'Sep', return: 1.5, benchmark: 1.2 },
    { month: 'Oct', return: 1.9, benchmark: 1.2 },
    { month: 'Nov', return: 10.0, benchmark: 1.3 }
  ]);

  const runBacktest = async () => {
    setIsRunning(true);
    
    // Simulate backtest running
    setTimeout(() => {
      const newResults = {
        totalReturn: 24.7,
        annualizedReturn: 24.7,
        volatility: 16.8,
        sharpeRatio: 1.42,
        sortinoRatio: 1.89,
        maxDrawdown: -3.1,
        calmarRatio: 7.97,
        var95: 2.1,
        cvar95: 3.2,
        totalTrades: 156,
        winningTrades: 98,
        losingTrades: 58,
        winRate: 62.8,
        avgWin: 2.4,
        avgLoss: -1.8,
        profitFactor: 1.67,
        beta: 1.15,
        alpha: 7.5,
        informationRatio: 1.68
      };
      
      setResults(newResults);
      setIsRunning(false);
      
      // Add to history
      const newHistoryItem = {
        id: backtestHistory.length + 1,
        strategy: config.strategy.charAt(0).toUpperCase() + config.strategy.slice(1) + ' Strategy',
        period: `${config.startDate} to ${config.endDate}`,
        return: newResults.totalReturn,
        sharpe: newResults.sharpeRatio,
        maxDD: newResults.maxDrawdown,
        trades: newResults.totalTrades,
        status: 'COMPLETED',
        date: new Date().toISOString().split('T')[0]
      };
      
      setBacktestHistory(prev => [newHistoryItem, ...prev]);
    }, 3000);
  };

  const formatPercent = (value) => `${value.toFixed(2)}%`;
  const formatNumber = (value) => value.toFixed(2);
  const formatCurrency = (value) => `$${value.toLocaleString()}`;

  return (
    <BacktestContainer>
      <BacktestHeader>🔬 Strategy Backtesting</BacktestHeader>
      
      {/* Configuration Panel */}
      <ConfigPanel>
        <h3>Backtest Configuration</h3>
        <ConfigGrid>
          <FormGroup>
            <label>Strategy</label>
            <select 
              value={config.strategy} 
              onChange={(e) => setConfig({...config, strategy: e.target.value})}
            >
              <option value="momentum">Momentum Strategy</option>
              <option value="options_straddle">Options Straddle</option>
              <option value="iron_condor">Iron Condor</option>
              <option value="covered_call">Covered Call</option>
              <option value="mean_reversion">Mean Reversion</option>
            </select>
          </FormGroup>
          
          <FormGroup>
            <label>Start Date</label>
            <input 
              type="date" 
              value={config.startDate}
              onChange={(e) => setConfig({...config, startDate: e.target.value})}
            />
          </FormGroup>
          
          <FormGroup>
            <label>End Date</label>
            <input 
              type="date" 
              value={config.endDate}
              onChange={(e) => setConfig({...config, endDate: e.target.value})}
            />
          </FormGroup>
          
          <FormGroup>
            <label>Initial Capital</label>
            <input 
              type="number" 
              value={config.initialCapital}
              onChange={(e) => setConfig({...config, initialCapital: parseInt(e.target.value)})}
              step="10000"
            />
          </FormGroup>
          
          <FormGroup>
            <label>Symbols (comma-separated)</label>
            <input 
              type="text" 
              value={config.symbols}
              onChange={(e) => setConfig({...config, symbols: e.target.value})}
              placeholder="AAPL,GOOGL,TSLA"
            />
          </FormGroup>
          
          <FormGroup>
            <label>Commission per Trade</label>
            <input 
              type="number" 
              value={config.commission}
              onChange={(e) => setConfig({...config, commission: parseFloat(e.target.value)})}
              step="0.1"
            />
          </FormGroup>
        </ConfigGrid>
        
        <div>
          <RunButton onClick={runBacktest} disabled={isRunning}>
            {isRunning ? 'Running Backtest...' : 'Run Backtest'}
          </RunButton>
          <RunButton style={{background: '#FF9800'}} disabled={isRunning}>
            Optimize Parameters
          </RunButton>
          <RunButton style={{background: '#9C27B0'}} disabled={isRunning}>
            Monte Carlo Analysis
          </RunButton>
        </div>
      </ConfigPanel>

      {/* Results */}
      {results && (
        <>
          <ResultsGrid>
            <ResultCard>
              <h3>Performance Metrics</h3>
              <MetricsGrid>
                <MetricItem positive={results.totalReturn > 0}>
                  <div className="metric-label">Total Return</div>
                  <div className="metric-value">{formatPercent(results.totalReturn)}</div>
                </MetricItem>
                <MetricItem positive={results.sharpeRatio > 1}>
                  <div className="metric-label">Sharpe Ratio</div>
                  <div className="metric-value">{formatNumber(results.sharpeRatio)}</div>
                </MetricItem>
                <MetricItem negative>
                  <div className="metric-label">Max Drawdown</div>
                  <div className="metric-value">{formatPercent(results.maxDrawdown)}</div>
                </MetricItem>
                <MetricItem>
                  <div className="metric-label">Volatility</div>
                  <div className="metric-value">{formatPercent(results.volatility)}</div>
                </MetricItem>
                <MetricItem positive={results.alpha > 0}>
                  <div className="metric-label">Alpha</div>
                  <div className="metric-value">{formatPercent(results.alpha)}</div>
                </MetricItem>
                <MetricItem>
                  <div className="metric-label">Beta</div>
                  <div className="metric-value">{formatNumber(results.beta)}</div>
                </MetricItem>
              </MetricsGrid>
            </ResultCard>

            <ResultCard>
              <h3>Trading Statistics</h3>
              <MetricsGrid>
                <MetricItem>
                  <div className="metric-label">Total Trades</div>
                  <div className="metric-value">{results.totalTrades}</div>
                </MetricItem>
                <MetricItem positive>
                  <div className="metric-label">Win Rate</div>
                  <div className="metric-value">{formatPercent(results.winRate)}</div>
                </MetricItem>
                <MetricItem positive>
                  <div className="metric-label">Profit Factor</div>
                  <div className="metric-value">{formatNumber(results.profitFactor)}</div>
                </MetricItem>
                <MetricItem positive>
                  <div className="metric-label">Avg Win</div>
                  <div className="metric-value">{formatPercent(results.avgWin)}</div>
                </MetricItem>
                <MetricItem negative>
                  <div className="metric-label">Avg Loss</div>
                  <div className="metric-value">{formatPercent(results.avgLoss)}</div>
                </MetricItem>
                <MetricItem>
                  <div className="metric-label">VaR (95%)</div>
                  <div className="metric-value">{formatPercent(results.var95)}</div>
                </MetricItem>
              </MetricsGrid>
            </ResultCard>
          </ResultsGrid>

          <ResultsGrid>
            <ResultCard>
              <h3>Equity Curve</h3>
              <ResponsiveContainer width="100%" height={300}>
                <LineChart data={equityCurve}>
                  <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.1)" />
                  <XAxis dataKey="date" stroke="#fff" />
                  <YAxis stroke="#fff" />
                  <Tooltip 
                    contentStyle={{ 
                      backgroundColor: 'rgba(0,0,0,0.8)', 
                      border: '1px solid rgba(255,255,255,0.2)',
                      borderRadius: '8px'
                    }}
                  />
                  <Line type="monotone" dataKey="value" stroke="#4CAF50" strokeWidth={2} name="Strategy" />
                  <Line type="monotone" dataKey="benchmark" stroke="#FF9800" strokeWidth={2} name="Benchmark" />
                </LineChart>
              </ResponsiveContainer>
            </ResultCard>

            <ResultCard>
              <h3>Monthly Returns</h3>
              <ResponsiveContainer width="100%" height={300}>
                <BarChart data={monthlyReturns}>
                  <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.1)" />
                  <XAxis dataKey="month" stroke="#fff" />
                  <YAxis stroke="#fff" />
                  <Tooltip 
                    contentStyle={{ 
                      backgroundColor: 'rgba(0,0,0,0.8)', 
                      border: '1px solid rgba(255,255,255,0.2)',
                      borderRadius: '8px'
                    }}
                  />
                  <Bar dataKey="return" fill="#4CAF50" name="Strategy" />
                  <Bar dataKey="benchmark" fill="#FF9800" name="Benchmark" />
                </BarChart>
              </ResponsiveContainer>
            </ResultCard>
          </ResultsGrid>

          <ResultCard>
            <h3>Risk-Return Scatter</h3>
            <ResponsiveContainer width="100%" height={250}>
              <ScatterChart data={[{return: results.totalReturn, risk: results.volatility, name: 'Strategy'}]}>
                <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.1)" />
                <XAxis dataKey="risk" stroke="#fff" name="Risk %" />
                <YAxis dataKey="return" stroke="#fff" name="Return %" />
                <Tooltip 
                  contentStyle={{ 
                    backgroundColor: 'rgba(0,0,0,0.8)', 
                    border: '1px solid rgba(255,255,255,0.2)',
                    borderRadius: '8px'
                  }}
                />
                <Scatter dataKey="return" fill="#4CAF50" />
              </ScatterChart>
            </ResponsiveContainer>
          </ResultCard>
        </>
      )}

      {/* Backtest History */}
      <ResultCard>
        <h3>Backtest History</h3>
        <ComparisonTable>
          <thead>
            <tr>
              <th>Date</th>
              <th>Strategy</th>
              <th>Period</th>
              <th>Return</th>
              <th>Sharpe</th>
              <th>Max DD</th>
              <th>Trades</th>
              <th>Status</th>
            </tr>
          </thead>
          <tbody>
            {backtestHistory.map((test) => (
              <tr key={test.id}>
                <td>{test.date}</td>
                <td><strong>{test.strategy}</strong></td>
                <td>{test.period}</td>
                <td style={{ color: test.return > 0 ? '#4CAF50' : '#f44336' }}>
                  {formatPercent(test.return)}
                </td>
                <td>{formatNumber(test.sharpe)}</td>
                <td style={{ color: '#f44336' }}>{formatPercent(test.maxDD)}</td>
                <td>{test.trades}</td>
                <td>
                  <StatusBadge status={test.status}>
                    {test.status}
                  </StatusBadge>
                </td>
              </tr>
            ))}
          </tbody>
        </ComparisonTable>
      </ResultCard>
    </BacktestContainer>
  );
};

export default Backtesting;