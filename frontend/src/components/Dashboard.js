import React, { useState, useEffect } from 'react';
import styled from 'styled-components';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';
import io from 'socket.io-client';

const DashboardContainer = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 20px;
  margin-bottom: 30px;
`;

const Card = styled.div`
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-radius: 15px;
  padding: 20px;
  border: 1px solid rgba(255, 255, 255, 0.2);
`;

const CardTitle = styled.h3`
  margin: 0 0 15px 0;
  color: #fff;
  font-size: 1.2em;
`;

const MetricValue = styled.div`
  font-size: 2em;
  font-weight: bold;
  color: ${props => props.positive ? '#4CAF50' : props.negative ? '#f44336' : '#fff'};
`;

const MetricLabel = styled.div`
  font-size: 0.9em;
  color: rgba(255, 255, 255, 0.7);
  margin-top: 5px;
`;

const ChartContainer = styled.div`
  height: 300px;
  margin-top: 20px;
`;

const AlertsContainer = styled.div`
  background: rgba(255, 255, 255, 0.1);
  border-radius: 15px;
  padding: 20px;
  margin-top: 20px;
`;

const Alert = styled.div`
  background: ${props => props.type === 'warning' ? 'rgba(255, 193, 7, 0.2)' : 'rgba(244, 67, 54, 0.2)'};
  border-left: 4px solid ${props => props.type === 'warning' ? '#FFC107' : '#f44336'};
  padding: 10px;
  margin: 10px 0;
  border-radius: 5px;
`;

function Dashboard() {
  const [portfolioValue, setPortfolioValue] = useState(1250000);
  const [dailyPnL, setDailyPnL] = useState(15420);
  const [totalReturn, setTotalReturn] = useState(8.7);
  const [sharpeRatio, setSharpeRatio] = useState(1.85);
  const [maxDrawdown, setMaxDrawdown] = useState(-3.2);
  const [var95, setVar95] = useState(-45000);
  const [priceData, setPriceData] = useState([]);
  const [alerts, setAlerts] = useState([]);

  useEffect(() => {
    // Initialize WebSocket connection for real-time updates
    const socket = io('ws://localhost:8080');
    
    socket.on('market_data', (data) => {
      setPriceData(prev => [...prev.slice(-50), {
        time: new Date().toLocaleTimeString(),
        price: data.price,
        volume: data.volume
      }]);
    });

    socket.on('portfolio_update', (data) => {
      setPortfolioValue(data.total_value);
      setDailyPnL(data.daily_pnl);
    });

    socket.on('risk_alert', (alert) => {
      setAlerts(prev => [alert, ...prev.slice(0, 4)]);
    });

    // Generate sample data
    const generateSampleData = () => {
      const data = [];
      let price = 150;
      for (let i = 0; i < 50; i++) {
        price += (Math.random() - 0.5) * 2;
        data.push({
          time: new Date(Date.now() - (50 - i) * 60000).toLocaleTimeString(),
          price: price.toFixed(2),
          volume: Math.floor(Math.random() * 1000) + 500
        });
      }
      setPriceData(data);
    };

    generateSampleData();

    // Sample alerts
    setAlerts([
      { type: 'warning', message: 'Portfolio approaching daily loss limit', timestamp: new Date() },
      { type: 'error', message: 'VaR threshold exceeded for TSLA position', timestamp: new Date() }
    ]);

    return () => socket.disconnect();
  }, []);

  return (
    <div>
      <h1>Trading Dashboard</h1>
      
      <DashboardContainer>
        <Card>
          <CardTitle>Portfolio Value</CardTitle>
          <MetricValue>${portfolioValue.toLocaleString()}</MetricValue>
          <MetricLabel>Total Assets Under Management</MetricLabel>
        </Card>

        <Card>
          <CardTitle>Daily P&L</CardTitle>
          <MetricValue positive={dailyPnL > 0} negative={dailyPnL < 0}>
            ${dailyPnL > 0 ? '+' : ''}${dailyPnL.toLocaleString()}
          </MetricValue>
          <MetricLabel>Today's Performance</MetricLabel>
        </Card>

        <Card>
          <CardTitle>Total Return</CardTitle>
          <MetricValue positive={totalReturn > 0}>
            {totalReturn > 0 ? '+' : ''}{totalReturn}%
          </MetricValue>
          <MetricLabel>Year to Date</MetricLabel>
        </Card>

        <Card>
          <CardTitle>Sharpe Ratio</CardTitle>
          <MetricValue>{sharpeRatio}</MetricValue>
          <MetricLabel>Risk-Adjusted Return</MetricLabel>
        </Card>

        <Card>
          <CardTitle>Max Drawdown</CardTitle>
          <MetricValue negative>{maxDrawdown}%</MetricValue>
          <MetricLabel>Maximum Loss from Peak</MetricLabel>
        </Card>

        <Card>
          <CardTitle>VaR (95%)</CardTitle>
          <MetricValue negative>${var95.toLocaleString()}</MetricValue>
          <MetricLabel>1-Day Value at Risk</MetricLabel>
        </Card>
      </DashboardContainer>

      <Card>
        <CardTitle>Real-Time Market Data</CardTitle>
        <ChartContainer>
          <ResponsiveContainer width="100%" height="100%">
            <LineChart data={priceData}>
              <CartesianGrid strokeDasharray="3,3" stroke="rgba(255,255,255,0.1)" />
              <XAxis dataKey="time" stroke="rgba(255,255,255,0.7)" />
              <YAxis stroke="rgba(255,255,255,0.7)" />
              <Tooltip 
                contentStyle={{ 
                  backgroundColor: 'rgba(0,0,0,0.8)', 
                  border: '1px solid rgba(255,255,255,0.2)',
                  borderRadius: '5px'
                }} 
              />
              <Line 
                type="monotone" 
                dataKey="price" 
                stroke="#4CAF50" 
                strokeWidth={2}
                dot={false}
              />
            </LineChart>
          </ResponsiveContainer>
        </ChartContainer>
      </Card>

      <AlertsContainer>
        <CardTitle>Risk Alerts</CardTitle>
        {alerts.map((alert, index) => (
          <Alert key={index} type={alert.type}>
            <strong>{alert.type === 'warning' ? '⚠️ Warning' : '🚨 Alert'}:</strong> {alert.message}
          </Alert>
        ))}
      </AlertsContainer>
    </div>
  );
}

export default Dashboard;