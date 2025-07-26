import React, { useState } from 'react';
import styled from 'styled-components';

const RiskContainer = styled.div`
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 20px;
`;

const Card = styled.div`
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-radius: 15px;
  padding: 20px;
  border: 1px solid rgba(255, 255, 255, 0.2);
`;

const FormGroup = styled.div`
  margin-bottom: 15px;
`;

const Label = styled.label`
  display: block;
  margin-bottom: 5px;
  color: rgba(255, 255, 255, 0.9);
`;

const Input = styled.input`
  width: 100%;
  padding: 10px;
  border: 1px solid rgba(255, 255, 255, 0.3);
  border-radius: 5px;
  background: rgba(255, 255, 255, 0.1);
  color: white;
`;

const Button = styled.button`
  padding: 12px 24px;
  border: none;
  border-radius: 5px;
  background: #4CAF50;
  color: white;
  font-weight: bold;
  cursor: pointer;
  
  &:hover {
    opacity: 0.9;
  }
`;

const RiskMetric = styled.div`
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 10px;
  margin: 10px 0;
  background: rgba(255, 255, 255, 0.05);
  border-radius: 5px;
  border-left: 4px solid ${props => 
    props.status === 'danger' ? '#f44336' : 
    props.status === 'warning' ? '#FFC107' : '#4CAF50'};
`;

function RiskManagement() {
  const [riskParams, setRiskParams] = useState({
    maxPositionSize: '100000',
    maxDailyLoss: '50000',
    maxPortfolioVaR: '100000',
    maxLeverage: '3',
    maxConcentration: '0.1',
    stopLossPercent: '0.05'
  });

  const [currentRisk, setCurrentRisk] = useState({
    portfolioVaR: 75000,
    dailyPnL: -15000,
    leverage: 2.1,
    concentration: 0.08,
    largestPosition: 85000
  });

  const handleParamChange = (param, value) => {
    setRiskParams(prev => ({
      ...prev,
      [param]: value
    }));
  };

  const saveRiskParams = () => {
    console.log('Saving risk parameters:', riskParams);
    alert('Risk parameters updated successfully!');
  };

  const getRiskStatus = (current, limit, isPercentage = false) => {
    const ratio = Math.abs(current) / Math.abs(limit);
    if (ratio > 0.9) return 'danger';
    if (ratio > 0.7) return 'warning';
    return 'safe';
  };

  return (
    <div>
      <h1>Risk Management</h1>
      
      <RiskContainer>
        <Card>
          <h3>Risk Parameters (User Configurable)</h3>
          
          <FormGroup>
            <Label>Max Position Size ($)</Label>
            <Input
              type="number"
              value={riskParams.maxPositionSize}
              onChange={(e) => handleParamChange('maxPositionSize', e.target.value)}
            />
          </FormGroup>

          <FormGroup>
            <Label>Max Daily Loss ($)</Label>
            <Input
              type="number"
              value={riskParams.maxDailyLoss}
              onChange={(e) => handleParamChange('maxDailyLoss', e.target.value)}
            />
          </FormGroup>

          <FormGroup>
            <Label>Max Portfolio VaR ($)</Label>
            <Input
              type="number"
              value={riskParams.maxPortfolioVaR}
              onChange={(e) => handleParamChange('maxPortfolioVaR', e.target.value)}
            />
          </FormGroup>

          <FormGroup>
            <Label>Max Leverage Ratio</Label>
            <Input
              type="number"
              step="0.1"
              value={riskParams.maxLeverage}
              onChange={(e) => handleParamChange('maxLeverage', e.target.value)}
            />
          </FormGroup>

          <FormGroup>
            <Label>Max Concentration (%)</Label>
            <Input
              type="number"
              step="0.01"
              value={riskParams.maxConcentration}
              onChange={(e) => handleParamChange('maxConcentration', e.target.value)}
            />
          </FormGroup>

          <FormGroup>
            <Label>Stop Loss Percentage (%)</Label>
            <Input
              type="number"
              step="0.01"
              value={riskParams.stopLossPercent}
              onChange={(e) => handleParamChange('stopLossPercent', e.target.value)}
            />
          </FormGroup>

          <Button onClick={saveRiskParams}>
            Update Risk Parameters
          </Button>
        </Card>

        <Card>
          <h3>Current Risk Metrics</h3>
          
          <RiskMetric status={getRiskStatus(currentRisk.portfolioVaR, riskParams.maxPortfolioVaR)}>
            <span>Portfolio VaR (95%)</span>
            <span>${currentRisk.portfolioVaR.toLocaleString()} / ${parseInt(riskParams.maxPortfolioVaR).toLocaleString()}</span>
          </RiskMetric>

          <RiskMetric status={getRiskStatus(currentRisk.dailyPnL, riskParams.maxDailyLoss)}>
            <span>Daily P&L</span>
            <span>${currentRisk.dailyPnL.toLocaleString()} / -${parseInt(riskParams.maxDailyLoss).toLocaleString()}</span>
          </RiskMetric>

          <RiskMetric status={getRiskStatus(currentRisk.leverage, riskParams.maxLeverage)}>
            <span>Current Leverage</span>
            <span>{currentRisk.leverage}x / {riskParams.maxLeverage}x</span>
          </RiskMetric>

          <RiskMetric status={getRiskStatus(currentRisk.concentration, riskParams.maxConcentration)}>
            <span>Max Concentration</span>
            <span>{(currentRisk.concentration * 100).toFixed(1)}% / {(parseFloat(riskParams.maxConcentration) * 100).toFixed(1)}%</span>
          </RiskMetric>

          <RiskMetric status={getRiskStatus(currentRisk.largestPosition, riskParams.maxPositionSize)}>
            <span>Largest Position</span>
            <span>${currentRisk.largestPosition.toLocaleString()} / ${parseInt(riskParams.maxPositionSize).toLocaleString()}</span>
          </RiskMetric>
        </Card>
      </RiskContainer>

      <Card style={{ marginTop: '20px' }}>
        <h3>Risk Actions</h3>
        <div style={{ display: 'flex', gap: '10px', flexWrap: 'wrap' }}>
          <Button onClick={() => alert('Calculating portfolio stress test...')}>
            Run Stress Test
          </Button>
          <Button onClick={() => alert('Generating risk report...')}>
            Generate Risk Report
          </Button>
          <Button onClick={() => alert('Rebalancing portfolio...')}>
            Auto Rebalance
          </Button>
          <Button onClick={() => alert('Emergency stop activated!')}>
            Emergency Stop
          </Button>
        </div>
      </Card>
    </div>
  );
}

export default RiskManagement;