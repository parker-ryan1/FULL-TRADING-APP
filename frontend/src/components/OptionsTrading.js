import React, { useState, useEffect } from 'react';
import styled from 'styled-components';

const OptionsContainer = styled.div`
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

const Select = styled.select`
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
  margin-right: 10px;
  
  &:hover {
    opacity: 0.9;
  }
`;

const PriceDisplay = styled.div`
  background: rgba(0, 0, 0, 0.3);
  padding: 15px;
  border-radius: 10px;
  margin: 15px 0;
`;

const PriceValue = styled.div`
  font-size: 1.5em;
  font-weight: bold;
  color: #4CAF50;
  margin-bottom: 10px;
`;

const GreeksContainer = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(100px, 1fr));
  gap: 10px;
  margin-top: 15px;
`;

const GreekItem = styled.div`
  text-align: center;
  padding: 10px;
  background: rgba(255, 255, 255, 0.05);
  border-radius: 5px;
`;

const GreekLabel = styled.div`
  font-size: 0.8em;
  color: rgba(255, 255, 255, 0.7);
`;

const GreekValue = styled.div`
  font-weight: bold;
  color: white;
`;

function OptionsTrading() {
  const [symbol, setSymbol] = useState('AAPL');
  const [strike, setStrike] = useState('155');
  const [expiry, setExpiry] = useState('2024-03-15');
  const [optionType, setOptionType] = useState('CALL');
  const [spotPrice, setSpotPrice] = useState('150.25');
  const [volatility, setVolatility] = useState('0.25');
  const [riskFreeRate, setRiskFreeRate] = useState('0.05');
  
  const [pricing, setPricing] = useState({
    blackScholes: 0,
    monteCarlo: 0,
    impliedVol: 0,
    greeks: {
      delta: 0,
      gamma: 0,
      theta: 0,
      vega: 0,
      rho: 0
    }
  });

  useEffect(() => {
    calculatePricing();
  }, [symbol, strike, expiry, optionType, spotPrice, volatility, riskFreeRate]);

  const calculatePricing = () => {
    // Simulate option pricing calculation
    const S = parseFloat(spotPrice);
    const K = parseFloat(strike);
    const vol = parseFloat(volatility);
    const r = parseFloat(riskFreeRate);
    
    // Simple Black-Scholes approximation for demo
    const timeToExpiry = (new Date(expiry) - new Date()) / (365 * 24 * 60 * 60 * 1000);
    const d1 = (Math.log(S / K) + (r + 0.5 * vol * vol) * timeToExpiry) / (vol * Math.sqrt(timeToExpiry));
    const d2 = d1 - vol * Math.sqrt(timeToExpiry);
    
    const normalCDF = (x) => 0.5 * (1 + erf(x / Math.sqrt(2)));
    const erf = (x) => {
      const a1 =  0.254829592;
      const a2 = -0.284496736;
      const a3 =  1.421413741;
      const a4 = -1.453152027;
      const a5 =  1.061405429;
      const p  =  0.3275911;
      const sign = x < 0 ? -1 : 1;
      x = Math.abs(x);
      const t = 1.0 / (1.0 + p * x);
      const y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * Math.exp(-x * x);
      return sign * y;
    };

    let bsPrice;
    if (optionType === 'CALL') {
      bsPrice = S * normalCDF(d1) - K * Math.exp(-r * timeToExpiry) * normalCDF(d2);
    } else {
      bsPrice = K * Math.exp(-r * timeToExpiry) * normalCDF(-d2) - S * normalCDF(-d1);
    }

    // Simulate Monte Carlo price (slightly different)
    const mcPrice = bsPrice * (0.95 + Math.random() * 0.1);

    // Calculate Greeks (simplified)
    const delta = optionType === 'CALL' ? normalCDF(d1) : normalCDF(d1) - 1;
    const gamma = Math.exp(-d1 * d1 / 2) / (S * vol * Math.sqrt(2 * Math.PI * timeToExpiry));
    const theta = -(S * Math.exp(-d1 * d1 / 2) * vol) / (2 * Math.sqrt(2 * Math.PI * timeToExpiry)) / 365;
    const vega = S * Math.exp(-d1 * d1 / 2) * Math.sqrt(timeToExpiry) / Math.sqrt(2 * Math.PI) / 100;
    const rho = optionType === 'CALL' ? 
      K * timeToExpiry * Math.exp(-r * timeToExpiry) * normalCDF(d2) / 100 :
      -K * timeToExpiry * Math.exp(-r * timeToExpiry) * normalCDF(-d2) / 100;

    setPricing({
      blackScholes: bsPrice,
      monteCarlo: mcPrice,
      impliedVol: vol,
      greeks: {
        delta: delta,
        gamma: gamma,
        theta: theta,
        vega: vega,
        rho: rho
      }
    });
  };

  const handleTrade = (action) => {
    alert(`${action} ${optionType} option: ${symbol} ${strike} ${expiry}`);
  };

  return (
    <div>
      <h1>Options Trading</h1>
      
      <OptionsContainer>
        <Card>
          <h3>Option Parameters</h3>
          
          <FormGroup>
            <Label>Underlying Symbol</Label>
            <Input
              type="text"
              value={symbol}
              onChange={(e) => setSymbol(e.target.value.toUpperCase())}
            />
          </FormGroup>

          <FormGroup>
            <Label>Strike Price</Label>
            <Input
              type="number"
              step="0.01"
              value={strike}
              onChange={(e) => setStrike(e.target.value)}
            />
          </FormGroup>

          <FormGroup>
            <Label>Expiration Date</Label>
            <Input
              type="date"
              value={expiry}
              onChange={(e) => setExpiry(e.target.value)}
            />
          </FormGroup>

          <FormGroup>
            <Label>Option Type</Label>
            <Select value={optionType} onChange={(e) => setOptionType(e.target.value)}>
              <option value="CALL">Call</option>
              <option value="PUT">Put</option>
            </Select>
          </FormGroup>

          <FormGroup>
            <Label>Spot Price</Label>
            <Input
              type="number"
              step="0.01"
              value={spotPrice}
              onChange={(e) => setSpotPrice(e.target.value)}
            />
          </FormGroup>

          <FormGroup>
            <Label>Volatility</Label>
            <Input
              type="number"
              step="0.01"
              value={volatility}
              onChange={(e) => setVolatility(e.target.value)}
            />
          </FormGroup>

          <FormGroup>
            <Label>Risk-Free Rate</Label>
            <Input
              type="number"
              step="0.001"
              value={riskFreeRate}
              onChange={(e) => setRiskFreeRate(e.target.value)}
            />
          </FormGroup>

          <div>
            <Button onClick={() => handleTrade('BUY')}>Buy Option</Button>
            <Button onClick={() => handleTrade('SELL')}>Sell Option</Button>
          </div>
        </Card>

        <Card>
          <h3>Option Pricing</h3>
          
          <PriceDisplay>
            <div>Black-Scholes Price:</div>
            <PriceValue>${pricing.blackScholes.toFixed(2)}</PriceValue>
          </PriceDisplay>

          <PriceDisplay>
            <div>Monte Carlo Price:</div>
            <PriceValue>${pricing.monteCarlo.toFixed(2)}</PriceValue>
          </PriceDisplay>

          <h4>The Greeks</h4>
          <GreeksContainer>
            <GreekItem>
              <GreekLabel>Delta</GreekLabel>
              <GreekValue>{pricing.greeks.delta.toFixed(4)}</GreekValue>
            </GreekItem>
            <GreekItem>
              <GreekLabel>Gamma</GreekLabel>
              <GreekValue>{pricing.greeks.gamma.toFixed(4)}</GreekValue>
            </GreekItem>
            <GreekItem>
              <GreekLabel>Theta</GreekLabel>
              <GreekValue>{pricing.greeks.theta.toFixed(4)}</GreekValue>
            </GreekItem>
            <GreekItem>
              <GreekLabel>Vega</GreekLabel>
              <GreekValue>{pricing.greeks.vega.toFixed(4)}</GreekValue>
            </GreekItem>
            <GreekItem>
              <GreekLabel>Rho</GreekLabel>
              <GreekValue>{pricing.greeks.rho.toFixed(4)}</GreekValue>
            </GreekItem>
          </GreeksContainer>
        </Card>
      </OptionsContainer>
    </div>
  );
}

export default OptionsTrading;