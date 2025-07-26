import React from 'react';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import { Elements } from '@stripe/react-stripe-js';
import { PayPalScriptProvider } from '@paypal/react-paypal-js';
import { loadStripe } from '@stripe/stripe-js';
import styled from 'styled-components';

import Header from './components/Header';
import Dashboard from './components/Dashboard';
import Trading from './components/Trading';
import Portfolio from './components/Portfolio';
import Analytics from './components/Analytics';
import RiskManagement from './components/RiskManagement';
import OptionsTrading from './components/OptionsTrading';
import AlgorithmicTrading from './components/AlgorithmicTrading';
import Backtesting from './components/Backtesting';
import Payment from './components/Payment';

const stripePromise = loadStripe('pk_test_your_stripe_publishable_key');

const AppContainer = styled.div`
  min-height: 100vh;
  background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
  color: white;
`;

const MainContent = styled.main`
  padding: 20px;
  max-width: 1400px;
  margin: 0 auto;
`;

function App() {
  return (
    <PayPalScriptProvider options={{ 
      "client-id": "your-paypal-client-id",
      currency: "USD"
    }}>
      <Elements stripe={stripePromise}>
        <AppContainer>
          <Router>
            <Header />
            <MainContent>
              <Routes>
                <Route path="/" element={<Dashboard />} />
                <Route path="/trading" element={<Trading />} />
                <Route path="/options" element={<OptionsTrading />} />
                <Route path="/algo" element={<AlgorithmicTrading />} />
                <Route path="/backtesting" element={<Backtesting />} />
                <Route path="/portfolio" element={<Portfolio />} />
                <Route path="/analytics" element={<Analytics />} />
                <Route path="/risk" element={<RiskManagement />} />
                <Route path="/payment" element={<Payment />} />
              </Routes>
            </MainContent>
          </Router>
        </AppContainer>
      </Elements>
    </PayPalScriptProvider>
  );
}

export default App;