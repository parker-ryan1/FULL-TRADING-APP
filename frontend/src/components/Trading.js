import React, { useState, useEffect } from 'react';
import styled from 'styled-components';

const TradingContainer = styled.div`
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 20px;
`;

const OrderPanel = styled.div`
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-radius: 15px;
  padding: 20px;
  border: 1px solid rgba(255, 255, 255, 0.2);
`;

const OrderBookPanel = styled.div`
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
  
  &::placeholder {
    color: rgba(255, 255, 255, 0.5);
  }
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
  background: ${props => props.variant === 'sell' ? '#f44336' : '#4CAF50'};
  color: white;
  font-weight: bold;
  cursor: pointer;
  margin-right: 10px;
  
  &:hover {
    opacity: 0.9;
  }
`;

const OrderBookTable = styled.table`
  width: 100%;
  border-collapse: collapse;
  margin-top: 10px;
`;

const OrderBookHeader = styled.th`
  padding: 10px;
  text-align: left;
  border-bottom: 1px solid rgba(255, 255, 255, 0.2);
  color: rgba(255, 255, 255, 0.9);
`;

const OrderBookRow = styled.tr`
  &:hover {
    background: rgba(255, 255, 255, 0.05);
  }
`;

const OrderBookCell = styled.td`
  padding: 8px 10px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.1);
  color: ${props => props.side === 'buy' ? '#4CAF50' : props.side === 'sell' ? '#f44336' : 'white'};
`;

function Trading() {
  const [symbol, setSymbol] = useState('AAPL');
  const [orderType, setOrderType] = useState('LIMIT');
  const [side, setSide] = useState('BUY');
  const [quantity, setQuantity] = useState('');
  const [price, setPrice] = useState('');
  const [orderBook, setOrderBook] = useState({ bids: [], asks: [] });
  const [recentTrades, setRecentTrades] = useState([]);

  useEffect(() => {
    // Simulate order book data
    const generateOrderBook = () => {
      const bids = [];
      const asks = [];
      let basePrice = 150;
      
      for (let i = 0; i < 10; i++) {
        bids.push({
          price: (basePrice - i * 0.01).toFixed(2),
          quantity: Math.floor(Math.random() * 1000) + 100,
          side: 'buy'
        });
        
        asks.push({
          price: (basePrice + 0.01 + i * 0.01).toFixed(2),
          quantity: Math.floor(Math.random() * 1000) + 100,
          side: 'sell'
        });
      }
      
      setOrderBook({ bids, asks });
    };

    // Simulate recent trades
    const generateRecentTrades = () => {
      const trades = [];
      let basePrice = 150;
      
      for (let i = 0; i < 20; i++) {
        trades.push({
          price: (basePrice + (Math.random() - 0.5) * 0.1).toFixed(2),
          quantity: Math.floor(Math.random() * 500) + 50,
          time: new Date(Date.now() - i * 30000).toLocaleTimeString(),
          side: Math.random() > 0.5 ? 'buy' : 'sell'
        });
      }
      
      setRecentTrades(trades);
    };

    generateOrderBook();
    generateRecentTrades();

    const interval = setInterval(() => {
      generateOrderBook();
      generateRecentTrades();
    }, 5000);

    return () => clearInterval(interval);
  }, [symbol]);

  const handleSubmitOrder = (orderSide) => {
    const order = {
      symbol,
      type: orderType,
      side: orderSide,
      quantity: parseFloat(quantity),
      price: orderType === 'MARKET' ? null : parseFloat(price)
    };

    console.log('Submitting order:', order);
    
    // In real implementation, would send to backend
    alert(`${orderSide} order submitted for ${quantity} shares of ${symbol}`);
    
    // Reset form
    setQuantity('');
    setPrice('');
  };

  return (
    <div>
      <h1>Trading Interface</h1>
      
      <TradingContainer>
        <OrderPanel>
          <h3>Place Order</h3>
          
          <FormGroup>
            <Label>Symbol</Label>
            <Input
              type="text"
              value={symbol}
              onChange={(e) => setSymbol(e.target.value.toUpperCase())}
              placeholder="Enter symbol (e.g., AAPL)"
            />
          </FormGroup>

          <FormGroup>
            <Label>Order Type</Label>
            <Select value={orderType} onChange={(e) => setOrderType(e.target.value)}>
              <option value="MARKET">Market</option>
              <option value="LIMIT">Limit</option>
              <option value="STOP">Stop</option>
              <option value="STOP_LIMIT">Stop Limit</option>
            </Select>
          </FormGroup>

          <FormGroup>
            <Label>Quantity</Label>
            <Input
              type="number"
              value={quantity}
              onChange={(e) => setQuantity(e.target.value)}
              placeholder="Number of shares"
            />
          </FormGroup>

          {orderType !== 'MARKET' && (
            <FormGroup>
              <Label>Price</Label>
              <Input
                type="number"
                step="0.01"
                value={price}
                onChange={(e) => setPrice(e.target.value)}
                placeholder="Price per share"
              />
            </FormGroup>
          )}

          <div>
            <Button onClick={() => handleSubmitOrder('BUY')}>
              Buy {symbol}
            </Button>
            <Button variant="sell" onClick={() => handleSubmitOrder('SELL')}>
              Sell {symbol}
            </Button>
          </div>
        </OrderPanel>

        <OrderBookPanel>
          <h3>Order Book - {symbol}</h3>
          
          <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '20px' }}>
            <div>
              <h4 style={{ color: '#4CAF50' }}>Bids</h4>
              <OrderBookTable>
                <thead>
                  <tr>
                    <OrderBookHeader>Price</OrderBookHeader>
                    <OrderBookHeader>Quantity</OrderBookHeader>
                  </tr>
                </thead>
                <tbody>
                  {orderBook.bids.map((bid, index) => (
                    <OrderBookRow key={index}>
                      <OrderBookCell side="buy">${bid.price}</OrderBookCell>
                      <OrderBookCell>{bid.quantity}</OrderBookCell>
                    </OrderBookRow>
                  ))}
                </tbody>
              </OrderBookTable>
            </div>

            <div>
              <h4 style={{ color: '#f44336' }}>Asks</h4>
              <OrderBookTable>
                <thead>
                  <tr>
                    <OrderBookHeader>Price</OrderBookHeader>
                    <OrderBookHeader>Quantity</OrderBookHeader>
                  </tr>
                </thead>
                <tbody>
                  {orderBook.asks.map((ask, index) => (
                    <OrderBookRow key={index}>
                      <OrderBookCell side="sell">${ask.price}</OrderBookCell>
                      <OrderBookCell>{ask.quantity}</OrderBookCell>
                    </OrderBookRow>
                  ))}
                </tbody>
              </OrderBookTable>
            </div>
          </div>
        </OrderBookPanel>
      </TradingContainer>

      <OrderBookPanel style={{ marginTop: '20px' }}>
        <h3>Recent Trades</h3>
        <OrderBookTable>
          <thead>
            <tr>
              <OrderBookHeader>Time</OrderBookHeader>
              <OrderBookHeader>Price</OrderBookHeader>
              <OrderBookHeader>Quantity</OrderBookHeader>
              <OrderBookHeader>Side</OrderBookHeader>
            </tr>
          </thead>
          <tbody>
            {recentTrades.map((trade, index) => (
              <OrderBookRow key={index}>
                <OrderBookCell>{trade.time}</OrderBookCell>
                <OrderBookCell side={trade.side}>${trade.price}</OrderBookCell>
                <OrderBookCell>{trade.quantity}</OrderBookCell>
                <OrderBookCell side={trade.side}>{trade.side.toUpperCase()}</OrderBookCell>
              </OrderBookRow>
            ))}
          </tbody>
        </OrderBookTable>
      </OrderBookPanel>
    </div>
  );
}

export default Trading;