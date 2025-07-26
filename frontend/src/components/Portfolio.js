import React, { useState, useEffect } from 'react';
import styled from 'styled-components';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, PieChart, Pie, Cell, BarChart, Bar } from 'recharts';

const PortfolioContainer = styled.div`
  padding: 20px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border-radius: 15px;
  color: white;
  margin: 20px 0;
`;

const PortfolioHeader = styled.h2`
  margin-bottom: 30px;
  text-align: center;
  font-size: 2.2em;
  text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
`;

const MetricsGrid = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 20px;
  margin-bottom: 30px;
`;

const MetricCard = styled.div`
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-radius: 12px;
  padding: 20px;
  text-align: center;
  border: 1px solid rgba(255, 255, 255, 0.2);
  
  .metric-label {
    font-size: 0.9em;
    opacity: 0.8;
    margin-bottom: 10px;
  }
  
  .metric-value {
    font-size: 1.8em;
    font-weight: bold;
    color: ${props => props.positive ? '#4CAF50' : props.negative ? '#f44336' : '#fff'};
  }
  
  .metric-change {
    font-size: 0.8em;
    margin-top: 5px;
    color: ${props => props.changePositive ? '#4CAF50' : '#f44336'};
  }
`;

const ChartsGrid = styled.div`
  display: grid;
  grid-template-columns: 1fr 1fr;
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

const PositionsTable = styled.div`
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-radius: 12px;
  padding: 20px;
  border: 1px solid rgba(255, 255, 255, 0.2);
  margin-bottom: 20px;
  
  h3 {
    margin-bottom: 20px;
  }
`;

const Table = styled.table`
  width: 100%;
  border-collapse: collapse;
  
  th, td {
    padding: 12px;
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

const StatusBadge = styled.span`
  padding: 4px 8px;
  border-radius: 12px;
  font-size: 0.8em;
  font-weight: bold;
  background: ${props => 
    props.status === 'PROFIT' ? '#4CAF50' : 
    props.status === 'LOSS' ? '#f44336' : 
    '#FF9800'
  };
  color: white;
`;

const COLORS = ['#0088FE', '#00C49F', '#FFBB28', '#FF8042', '#8884D8', '#82CA9D'];

const Portfolio = () => {
  const [portfolioData, setPortfolioData] = useState({
    totalValue: 1247850.75,
    totalPnL: 47850.75,
    dayPnL: 2340.50,
    totalReturn: 3.99,
    dayReturn: 0.19,
    sharpeRatio: 1.42,
    maxDrawdown: -2.8,
    beta: 1.15
  });

  const [positions, setPositions] = useState([
    {
      symbol: 'AAPL',
      quantity: 500,
      avgPrice: 148.50,
      currentPrice: 152.25,
      marketValue: 76125,
      unrealizedPnL: 1875,
      weight: 6.1,
      type: 'STOCK'
    },
    {
      symbol: 'GOOGL',
      quantity: 50,
      avgPrice: 2780.00,
      currentPrice: 2825.50,
      marketValue: 141275,
      unrealizedPnL: 2275,
      weight: 11.3,
      type: 'STOCK'
    },
    {
      symbol: 'TSLA',
      quantity: 200,
      avgPrice: 245.75,
      currentPrice: 238.90,
      marketValue: 47780,
      unrealizedPnL: -1370,
      weight: 3.8,
      type: 'STOCK'
    },
    {
      symbol: 'AAPL_CALL_155',
      quantity: 10,
      avgPrice: 8.50,
      currentPrice: 12.25,
      marketValue: 12250,
      unrealizedPnL: 3750,
      weight: 1.0,
      type: 'OPTION',
      strike: 155,
      expiration: '2024-03-15',
      delta: 0.65
    },
    {
      symbol: 'SPY_PUT_420',
      quantity: 25,
      avgPrice: 6.20,
      currentPrice: 4.80,
      marketValue: 12000,
      unrealizedPnL: -3500,
      weight: 1.0,
      type: 'OPTION',
      strike: 420,
      expiration: '2024-02-16',
      delta: -0.35
    }
  ]);

  const [performanceData, setPerformanceData] = useState([
    { date: '2024-01-01', value: 1200000, benchmark: 1200000 },
    { date: '2024-01-15', value: 1215000, benchmark: 1208000 },
    { date: '2024-02-01', value: 1198000, benchmark: 1195000 },
    { date: '2024-02-15', value: 1225000, benchmark: 1210000 },
    { date: '2024-03-01', value: 1240000, benchmark: 1220000 },
    { date: '2024-03-15', value: 1247850, benchmark: 1225000 }
  ]);

  const [allocationData, setAllocationData] = useState([
    { name: 'Technology', value: 45.2, color: '#0088FE' },
    { name: 'Healthcare', value: 18.7, color: '#00C49F' },
    { name: 'Financial', value: 15.3, color: '#FFBB28' },
    { name: 'Consumer', value: 12.1, color: '#FF8042' },
    { name: 'Options', value: 5.8, color: '#8884D8' },
    { name: 'Cash', value: 2.9, color: '#82CA9D' }
  ]);

  const [riskMetrics, setRiskMetrics] = useState([
    { metric: 'VaR (95%)', value: 2.1, limit: 3.0 },
    { metric: 'Beta', value: 1.15, limit: 1.5 },
    { metric: 'Leverage', value: 1.8, limit: 3.0 },
    { metric: 'Concentration', value: 11.3, limit: 15.0 }
  ]);

  useEffect(() => {
    // Simulate real-time updates
    const interval = setInterval(() => {
      setPortfolioData(prev => ({
        ...prev,
        totalValue: prev.totalValue + (Math.random() - 0.5) * 1000,
        dayPnL: prev.dayPnL + (Math.random() - 0.5) * 100,
        dayReturn: prev.dayReturn + (Math.random() - 0.5) * 0.1
      }));

      setPositions(prev => prev.map(pos => ({
        ...pos,
        currentPrice: pos.currentPrice + (Math.random() - 0.5) * 2,
        unrealizedPnL: pos.unrealizedPnL + (Math.random() - 0.5) * 50
      })));
    }, 3000);

    return () => clearInterval(interval);
  }, []);

  const formatCurrency = (value) => {
    return new Intl.NumberFormat('en-US', {
      style: 'currency',
      currency: 'USD'
    }).format(value);
  };

  const formatPercent = (value) => {
    return `${value.toFixed(2)}%`;
  };

  return (
    <PortfolioContainer>
      <PortfolioHeader>📊 Portfolio Management</PortfolioHeader>
      
      {/* Key Metrics */}
      <MetricsGrid>
        <MetricCard positive={portfolioData.totalPnL > 0}>
          <div className="metric-label">Total Portfolio Value</div>
          <div className="metric-value">{formatCurrency(portfolioData.totalValue)}</div>
          <div className="metric-change" changePositive={portfolioData.dayPnL > 0}>
            Today: {formatCurrency(portfolioData.dayPnL)} ({formatPercent(portfolioData.dayReturn)})
          </div>
        </MetricCard>
        
        <MetricCard positive={portfolioData.totalPnL > 0}>
          <div className="metric-label">Total P&L</div>
          <div className="metric-value">{formatCurrency(portfolioData.totalPnL)}</div>
          <div className="metric-change" changePositive={portfolioData.totalReturn > 0}>
            Return: {formatPercent(portfolioData.totalReturn)}
          </div>
        </MetricCard>
        
        <MetricCard>
          <div className="metric-label">Sharpe Ratio</div>
          <div className="metric-value">{portfolioData.sharpeRatio.toFixed(2)}</div>
          <div className="metric-change">Risk-Adjusted Return</div>
        </MetricCard>
        
        <MetricCard negative={portfolioData.maxDrawdown < 0}>
          <div className="metric-label">Max Drawdown</div>
          <div className="metric-value">{formatPercent(portfolioData.maxDrawdown)}</div>
          <div className="metric-change">Peak to Trough</div>
        </MetricCard>
        
        <MetricCard>
          <div className="metric-label">Beta</div>
          <div className="metric-value">{portfolioData.beta.toFixed(2)}</div>
          <div className="metric-change">vs S&P 500</div>
        </MetricCard>
      </MetricsGrid>

      {/* Charts */}
      <ChartsGrid>
        <ChartCard>
          <h3>Portfolio Performance vs Benchmark</h3>
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
              <Line type="monotone" dataKey="value" stroke="#4CAF50" strokeWidth={2} name="Portfolio" />
              <Line type="monotone" dataKey="benchmark" stroke="#FF9800" strokeWidth={2} name="S&P 500" />
            </LineChart>
          </ResponsiveContainer>
        </ChartCard>

        <ChartCard>
          <h3>Asset Allocation</h3>
          <ResponsiveContainer width="100%" height={300}>
            <PieChart>
              <Pie
                data={allocationData}
                cx="50%"
                cy="50%"
                labelLine={false}
                label={({ name, value }) => `${name}: ${value}%`}
                outerRadius={80}
                fill="#8884d8"
                dataKey="value"
              >
                {allocationData.map((entry, index) => (
                  <Cell key={`cell-${index}`} fill={COLORS[index % COLORS.length]} />
                ))}
              </Pie>
              <Tooltip />
            </PieChart>
          </ResponsiveContainer>
        </ChartCard>
      </ChartsGrid>

      {/* Risk Metrics Chart */}
      <ChartCard style={{ marginBottom: '20px' }}>
        <h3>Risk Metrics vs Limits</h3>
        <ResponsiveContainer width="100%" height={250}>
          <BarChart data={riskMetrics}>
            <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.1)" />
            <XAxis dataKey="metric" stroke="#fff" />
            <YAxis stroke="#fff" />
            <Tooltip 
              contentStyle={{ 
                backgroundColor: 'rgba(0,0,0,0.8)', 
                border: '1px solid rgba(255,255,255,0.2)',
                borderRadius: '8px'
              }}
            />
            <Bar dataKey="value" fill="#4CAF50" name="Current" />
            <Bar dataKey="limit" fill="#f44336" name="Limit" />
          </BarChart>
        </ResponsiveContainer>
      </ChartCard>

      {/* Positions Table */}
      <PositionsTable>
        <h3>Current Positions</h3>
        <Table>
          <thead>
            <tr>
              <th>Symbol</th>
              <th>Type</th>
              <th>Quantity</th>
              <th>Avg Price</th>
              <th>Current Price</th>
              <th>Market Value</th>
              <th>Unrealized P&L</th>
              <th>Weight</th>
              <th>Status</th>
            </tr>
          </thead>
          <tbody>
            {positions.map((position, index) => (
              <tr key={index}>
                <td>
                  <strong>{position.symbol}</strong>
                  {position.type === 'OPTION' && (
                    <div style={{ fontSize: '0.8em', opacity: '0.8' }}>
                      Strike: ${position.strike} | Exp: {position.expiration}
                      {position.delta && ` | Δ: ${position.delta.toFixed(2)}`}
                    </div>
                  )}
                </td>
                <td>{position.type}</td>
                <td>{position.quantity}</td>
                <td>{formatCurrency(position.avgPrice)}</td>
                <td>{formatCurrency(position.currentPrice)}</td>
                <td>{formatCurrency(position.marketValue)}</td>
                <td style={{ color: position.unrealizedPnL > 0 ? '#4CAF50' : '#f44336' }}>
                  {formatCurrency(position.unrealizedPnL)}
                </td>
                <td>{formatPercent(position.weight)}</td>
                <td>
                  <StatusBadge status={position.unrealizedPnL > 0 ? 'PROFIT' : 'LOSS'}>
                    {position.unrealizedPnL > 0 ? 'PROFIT' : 'LOSS'}
                  </StatusBadge>
                </td>
              </tr>
            ))}
          </tbody>
        </Table>
      </PositionsTable>
    </PortfolioContainer>
  );
};

export default Portfolio;