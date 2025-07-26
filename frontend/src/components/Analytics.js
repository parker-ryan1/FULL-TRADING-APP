import React, { useState, useEffect } from 'react';
import styled from 'styled-components';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, BarChart, Bar, ScatterChart, Scatter, AreaChart, Area } from 'recharts';

const AnalyticsContainer = styled.div`
  padding: 20px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border-radius: 15px;
  color: white;
  margin: 20px 0;
`;

const AnalyticsHeader = styled.h2`
  margin-bottom: 30px;
  text-align: center;
  font-size: 2.2em;
  text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
`;

const TabContainer = styled.div`
  display: flex;
  margin-bottom: 30px;
  background: rgba(255, 255, 255, 0.1);
  border-radius: 12px;
  padding: 5px;
`;

const Tab = styled.button`
  flex: 1;
  padding: 12px 20px;
  border: none;
  background: ${props => props.active ? 'rgba(255, 255, 255, 0.2)' : 'transparent'};
  color: white;
  border-radius: 8px;
  cursor: pointer;
  font-weight: ${props => props.active ? 'bold' : 'normal'};
  transition: all 0.3s ease;

  &:hover {
    background: rgba(255, 255, 255, 0.1);
  }
`;

const ChartsGrid = styled.div`
  display: grid;
  grid-template-columns: ${props => props.columns || '1fr 1fr'};
  gap: 20px;
  margin-bottom: 30px;
  
  @media (max-width: 768px) {
    grid-template-columns: 1fr;
  }
`;

const ChartCard = styled.div`
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
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 15px;
  margin-bottom: 30px;
`;

const MetricCard = styled.div`
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-radius: 12px;
  padding: 15px;
  text-align: center;
  border: 1px solid rgba(255, 255, 255, 0.2);
  
  .metric-label {
    font-size: 0.9em;
    opacity: 0.8;
    margin-bottom: 8px;
  }
  
  .metric-value {
    font-size: 1.5em;
    font-weight: bold;
    color: ${props => props.positive ? '#4CAF50' : props.negative ? '#f44336' : '#fff'};
  }
`;

const BacktestResults = styled.div`
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-radius: 12px;
  padding: 20px;
  border: 1px solid rgba(255, 255, 255, 0.2);
  margin-bottom: 20px;
`;

const ResultsTable = styled.table`
  width: 100%;
  border-collapse: collapse;
  margin-top: 15px;
  
  th, td {
    padding: 10px;
    text-align: left;
    border-bottom: 1px solid rgba(255, 255, 255, 0.1);
  }
  
  th {
    background: rgba(255, 255, 255, 0.1);
    font-weight: bold;
  }
  
  tr:hover {
    background: rgba(255, 255, 255, 0.05);
  }
`;

const Analytics = () => {
  const [activeTab, setActiveTab] = useState('performance');

  // Performance Analytics Data
  const [performanceData, setPerformanceData] = useState([
    { date: '2024-01', portfolio: 8.2, benchmark: 6.1, alpha: 2.1 },
    { date: '2024-02', portfolio: -2.1, benchmark: -1.8, alpha: -0.3 },
    { date: '2024-03', portfolio: 12.5, benchmark: 8.9, alpha: 3.6 },
    { date: '2024-04', portfolio: 5.8, benchmark: 4.2, alpha: 1.6 },
    { date: '2024-05', portfolio: -1.2, benchmark: 0.5, alpha: -1.7 },
    { date: '2024-06', portfolio: 9.3, benchmark: 7.1, alpha: 2.2 }
  ]);

  // Risk Analytics Data
  const [riskData, setRiskData] = useState([
    { date: '2024-01', var95: 2.1, var99: 3.2, volatility: 18.5 },
    { date: '2024-02', var95: 2.8, var99: 4.1, volatility: 22.3 },
    { date: '2024-03', var95: 1.9, var99: 2.7, volatility: 16.8 },
    { date: '2024-04', var95: 2.3, var99: 3.4, volatility: 19.2 },
    { date: '2024-05', var95: 2.6, var99: 3.8, volatility: 21.1 },
    { date: '2024-06', var95: 2.0, var99: 2.9, volatility: 17.4 }
  ]);

  // Strategy Performance Data
  const [strategyData, setStrategyData] = useState([
    { strategy: 'Momentum', return: 15.2, sharpe: 1.42, maxDD: -3.1, trades: 156 },
    { strategy: 'Options Straddle', return: 22.8, sharpe: 1.68, maxDD: -2.8, trades: 89 },
    { strategy: 'Iron Condor', return: 8.9, sharpe: 1.15, maxDD: -1.9, trades: 234 },
    { strategy: 'Covered Call', return: 12.4, sharpe: 1.89, maxDD: -1.2, trades: 67 },
    { strategy: 'Mean Reversion', return: 9.7, sharpe: 1.23, maxDD: -2.4, trades: 198 }
  ]);

  // Correlation Data
  const [correlationData, setCorrelationData] = useState([
    { asset1: 'AAPL', asset2: 'GOOGL', correlation: 0.72 },
    { asset1: 'AAPL', asset2: 'TSLA', correlation: 0.45 },
    { asset1: 'AAPL', asset2: 'SPY', correlation: 0.83 },
    { asset1: 'GOOGL', asset2: 'TSLA', correlation: 0.38 },
    { asset1: 'GOOGL', asset2: 'SPY', correlation: 0.79 },
    { asset1: 'TSLA', asset2: 'SPY', correlation: 0.51 }
  ]);

  // Drawdown Data
  const [drawdownData, setDrawdownData] = useState([
    { date: '2024-01-01', drawdown: 0 },
    { date: '2024-01-15', drawdown: -0.8 },
    { date: '2024-02-01', drawdown: -2.1 },
    { date: '2024-02-15', drawdown: -1.2 },
    { date: '2024-03-01', drawdown: 0 },
    { date: '2024-03-15', drawdown: -0.5 },
    { date: '2024-04-01', drawdown: -1.8 },
    { date: '2024-04-15', drawdown: -0.9 },
    { date: '2024-05-01', drawdown: -2.8 },
    { date: '2024-05-15', drawdown: -1.5 },
    { date: '2024-06-01', drawdown: 0 }
  ]);

  // Options Analytics Data
  const [optionsData, setOptionsData] = useState([
    { strike: 140, calls: 2.5, puts: 8.2, iv: 0.28 },
    { strike: 145, calls: 4.8, puts: 6.1, iv: 0.25 },
    { strike: 150, calls: 7.2, puts: 4.5, iv: 0.22 },
    { strike: 155, calls: 10.1, puts: 3.2, iv: 0.24 },
    { strike: 160, calls: 13.5, puts: 2.1, iv: 0.27 }
  ]);

  const formatPercent = (value) => `${value.toFixed(2)}%`;
  const formatNumber = (value) => value.toFixed(2);

  const renderPerformanceTab = () => (
    <>
      <MetricsGrid>
        <MetricCard positive>
          <div className="metric-label">Total Return</div>
          <div className="metric-value">24.7%</div>
        </MetricCard>
        <MetricCard positive>
          <div className="metric-label">Alpha</div>
          <div className="metric-value">7.5%</div>
        </MetricCard>
        <MetricCard>
          <div className="metric-label">Beta</div>
          <div className="metric-value">1.15</div>
        </MetricCard>
        <MetricCard positive>
          <div className="metric-label">Information Ratio</div>
          <div className="metric-value">1.68</div>
        </MetricCard>
        <MetricCard>
          <div className="metric-label">Tracking Error</div>
          <div className="metric-value">4.2%</div>
        </MetricCard>
        <MetricCard positive>
          <div className="metric-label">Calmar Ratio</div>
          <div className="metric-value">8.82</div>
        </MetricCard>
      </MetricsGrid>

      <ChartsGrid>
        <ChartCard>
          <h3>Monthly Returns vs Benchmark</h3>
          <ResponsiveContainer width="100%" height={300}>
            <BarChart data={performanceData}>
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
              <Bar dataKey="portfolio" fill="#4CAF50" name="Portfolio" />
              <Bar dataKey="benchmark" fill="#FF9800" name="S&P 500" />
            </BarChart>
          </ResponsiveContainer>
        </ChartCard>

        <ChartCard>
          <h3>Alpha Generation</h3>
          <ResponsiveContainer width="100%" height={300}>
            <LineChart data={performanceData}>
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
              <Line type="monotone" dataKey="alpha" stroke="#2196F3" strokeWidth={3} name="Alpha %" />
            </LineChart>
          </ResponsiveContainer>
        </ChartCard>
      </ChartsGrid>

      <ChartCard>
        <h3>Drawdown Analysis</h3>
        <ResponsiveContainer width="100%" height={250}>
          <AreaChart data={drawdownData}>
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
            <Area type="monotone" dataKey="drawdown" stroke="#f44336" fill="#f44336" fillOpacity={0.3} />
          </AreaChart>
        </ResponsiveContainer>
      </ChartCard>
    </>
  );

  const renderRiskTab = () => (
    <>
      <MetricsGrid>
        <MetricCard>
          <div className="metric-label">Portfolio VaR (95%)</div>
          <div className="metric-value">2.1%</div>
        </MetricCard>
        <MetricCard>
          <div className="metric-label">Expected Shortfall</div>
          <div className="metric-value">3.2%</div>
        </MetricCard>
        <MetricCard>
          <div className="metric-label">Volatility</div>
          <div className="metric-value">17.4%</div>
        </MetricCard>
        <MetricCard>
          <div className="metric-label">Skewness</div>
          <div className="metric-value">-0.15</div>
        </MetricCard>
        <MetricCard>
          <div className="metric-label">Kurtosis</div>
          <div className="metric-value">3.42</div>
        </MetricCard>
        <MetricCard>
          <div className="metric-label">Tail Ratio</div>
          <div className="metric-value">1.18</div>
        </MetricCard>
      </MetricsGrid>

      <ChartsGrid>
        <ChartCard>
          <h3>VaR Trends</h3>
          <ResponsiveContainer width="100%" height={300}>
            <LineChart data={riskData}>
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
              <Line type="monotone" dataKey="var95" stroke="#FF9800" strokeWidth={2} name="VaR 95%" />
              <Line type="monotone" dataKey="var99" stroke="#f44336" strokeWidth={2} name="VaR 99%" />
            </LineChart>
          </ResponsiveContainer>
        </ChartCard>

        <ChartCard>
          <h3>Volatility Analysis</h3>
          <ResponsiveContainer width="100%" height={300}>
            <AreaChart data={riskData}>
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
              <Area type="monotone" dataKey="volatility" stroke="#9C27B0" fill="#9C27B0" fillOpacity={0.3} />
            </AreaChart>
          </ResponsiveContainer>
        </ChartCard>
      </ChartsGrid>

      <ChartCard>
        <h3>Asset Correlation Matrix</h3>
        <ResponsiveContainer width="100%" height={250}>
          <ScatterChart data={correlationData}>
            <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.1)" />
            <XAxis dataKey="asset1" stroke="#fff" />
            <YAxis dataKey="correlation" stroke="#fff" />
            <Tooltip 
              contentStyle={{ 
                backgroundColor: 'rgba(0,0,0,0.8)', 
                border: '1px solid rgba(255,255,255,0.2)',
                borderRadius: '8px'
              }}
            />
            <Scatter dataKey="correlation" fill="#4CAF50" />
          </ScatterChart>
        </ResponsiveContainer>
      </ChartCard>
    </>
  );

  const renderStrategyTab = () => (
    <>
      <BacktestResults>
        <h3>Strategy Performance Comparison</h3>
        <ResultsTable>
          <thead>
            <tr>
              <th>Strategy</th>
              <th>Total Return</th>
              <th>Sharpe Ratio</th>
              <th>Max Drawdown</th>
              <th>Total Trades</th>
              <th>Win Rate</th>
            </tr>
          </thead>
          <tbody>
            {strategyData.map((strategy, index) => (
              <tr key={index}>
                <td><strong>{strategy.strategy}</strong></td>
                <td style={{ color: strategy.return > 0 ? '#4CAF50' : '#f44336' }}>
                  {formatPercent(strategy.return)}
                </td>
                <td>{formatNumber(strategy.sharpe)}</td>
                <td style={{ color: '#f44336' }}>{formatPercent(strategy.maxDD)}</td>
                <td>{strategy.trades}</td>
                <td>{formatPercent(65 + Math.random() * 20)}</td>
              </tr>
            ))}
          </tbody>
        </ResultsTable>
      </BacktestResults>

      <ChartsGrid>
        <ChartCard>
          <h3>Strategy Returns</h3>
          <ResponsiveContainer width="100%" height={300}>
            <BarChart data={strategyData}>
              <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.1)" />
              <XAxis dataKey="strategy" stroke="#fff" />
              <YAxis stroke="#fff" />
              <Tooltip 
                contentStyle={{ 
                  backgroundColor: 'rgba(0,0,0,0.8)', 
                  border: '1px solid rgba(255,255,255,0.2)',
                  borderRadius: '8px'
                }}
              />
              <Bar dataKey="return" fill="#4CAF50" />
            </BarChart>
          </ResponsiveContainer>
        </ChartCard>

        <ChartCard>
          <h3>Risk-Adjusted Performance</h3>
          <ResponsiveContainer width="100%" height={300}>
            <ScatterChart data={strategyData}>
              <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.1)" />
              <XAxis dataKey="return" stroke="#fff" name="Return" />
              <YAxis dataKey="sharpe" stroke="#fff" name="Sharpe" />
              <Tooltip 
                contentStyle={{ 
                  backgroundColor: 'rgba(0,0,0,0.8)', 
                  border: '1px solid rgba(255,255,255,0.2)',
                  borderRadius: '8px'
                }}
              />
              <Scatter dataKey="sharpe" fill="#2196F3" />
            </ScatterChart>
          </ResponsiveContainer>
        </ChartCard>
      </ChartsGrid>
    </>
  );

  const renderOptionsTab = () => (
    <>
      <MetricsGrid>
        <MetricCard>
          <div className="metric-label">Portfolio Delta</div>
          <div className="metric-value">+125.8</div>
        </MetricCard>
        <MetricCard>
          <div className="metric-label">Portfolio Gamma</div>
          <div className="metric-value">+8.2</div>
        </MetricCard>
        <MetricCard negative>
          <div className="metric-label">Portfolio Theta</div>
          <div className="metric-value">-45.6</div>
        </MetricCard>
        <MetricCard>
          <div className="metric-label">Portfolio Vega</div>
          <div className="metric-value">+89.3</div>
        </MetricCard>
        <MetricCard>
          <div className="metric-label">Implied Volatility</div>
          <div className="metric-value">24.8%</div>
        </MetricCard>
        <MetricCard>
          <div className="metric-label">Options P&L</div>
          <div className="metric-value">+$12,450</div>
        </MetricCard>
      </MetricsGrid>

      <ChartsGrid>
        <ChartCard>
          <h3>Options Pricing by Strike</h3>
          <ResponsiveContainer width="100%" height={300}>
            <LineChart data={optionsData}>
              <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.1)" />
              <XAxis dataKey="strike" stroke="#fff" />
              <YAxis stroke="#fff" />
              <Tooltip 
                contentStyle={{ 
                  backgroundColor: 'rgba(0,0,0,0.8)', 
                  border: '1px solid rgba(255,255,255,0.2)',
                  borderRadius: '8px'
                }}
              />
              <Line type="monotone" dataKey="calls" stroke="#4CAF50" strokeWidth={2} name="Calls" />
              <Line type="monotone" dataKey="puts" stroke="#f44336" strokeWidth={2} name="Puts" />
            </LineChart>
          </ResponsiveContainer>
        </ChartCard>

        <ChartCard>
          <h3>Implied Volatility Surface</h3>
          <ResponsiveContainer width="100%" height={300}>
            <AreaChart data={optionsData}>
              <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.1)" />
              <XAxis dataKey="strike" stroke="#fff" />
              <YAxis stroke="#fff" />
              <Tooltip 
                contentStyle={{ 
                  backgroundColor: 'rgba(0,0,0,0.8)', 
                  border: '1px solid rgba(255,255,255,0.2)',
                  borderRadius: '8px'
                }}
              />
              <Area type="monotone" dataKey="iv" stroke="#9C27B0" fill="#9C27B0" fillOpacity={0.3} />
            </AreaChart>
          </ResponsiveContainer>
        </ChartCard>
      </ChartsGrid>
    </>
  );

  return (
    <AnalyticsContainer>
      <AnalyticsHeader>📈 Advanced Analytics</AnalyticsHeader>
      
      <TabContainer>
        <Tab active={activeTab === 'performance'} onClick={() => setActiveTab('performance')}>
          Performance
        </Tab>
        <Tab active={activeTab === 'risk'} onClick={() => setActiveTab('risk')}>
          Risk Analysis
        </Tab>
        <Tab active={activeTab === 'strategy'} onClick={() => setActiveTab('strategy')}>
          Strategy Comparison
        </Tab>
        <Tab active={activeTab === 'options'} onClick={() => setActiveTab('options')}>
          Options Analytics
        </Tab>
      </TabContainer>

      {activeTab === 'performance' && renderPerformanceTab()}
      {activeTab === 'risk' && renderRiskTab()}
      {activeTab === 'strategy' && renderStrategyTab()}
      {activeTab === 'options' && renderOptionsTab()}
    </AnalyticsContainer>
  );
};

export default Analytics;