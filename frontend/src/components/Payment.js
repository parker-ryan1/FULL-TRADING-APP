import React, { useState } from 'react';
import styled from 'styled-components';
import { Elements, CardElement, useStripe, useElements } from '@stripe/react-stripe-js';
import { PayPalButtons, PayPalScriptProvider } from '@paypal/react-paypal-js';

const PaymentContainer = styled.div`
  padding: 20px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border-radius: 15px;
  color: white;
  margin: 20px 0;
`;

const PaymentHeader = styled.h2`
  margin-bottom: 30px;
  text-align: center;
  font-size: 2.2em;
  text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
`;

const PaymentGrid = styled.div`
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 30px;
  margin-bottom: 30px;
  
  @media (max-width: 768px) {
    grid-template-columns: 1fr;
  }
`;

const PaymentCard = styled.div`
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-radius: 12px;
  padding: 25px;
  border: 1px solid rgba(255, 255, 255, 0.2);
  
  h3 {
    margin-bottom: 20px;
    text-align: center;
    color: #fff;
  }
`;

const AccountSummary = styled.div`
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-radius: 12px;
  padding: 25px;
  border: 1px solid rgba(255, 255, 255, 0.2);
  margin-bottom: 30px;
`;

const SummaryGrid = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 20px;
  margin-bottom: 20px;
`;

const SummaryItem = styled.div`
  text-align: center;
  
  .label {
    font-size: 0.9em;
    opacity: 0.8;
    margin-bottom: 8px;
  }
  
  .value {
    font-size: 1.6em;
    font-weight: bold;
    color: ${props => props.positive ? '#4CAF50' : props.negative ? '#f44336' : '#fff'};
  }
`;

const FormGroup = styled.div`
  margin-bottom: 20px;
  
  label {
    display: block;
    margin-bottom: 8px;
    font-weight: 500;
  }
  
  input, select {
    width: 100%;
    padding: 12px;
    border: 1px solid rgba(255, 255, 255, 0.3);
    border-radius: 8px;
    background: rgba(255, 255, 255, 0.1);
    color: white;
    font-size: 16px;
    
    &::placeholder {
      color: rgba(255, 255, 255, 0.6);
    }
    
    &:focus {
      outline: none;
      border-color: #4CAF50;
      box-shadow: 0 0 0 2px rgba(76, 175, 80, 0.2);
    }
  }
`;

const PaymentButton = styled.button`
  width: 100%;
  padding: 15px;
  border: none;
  border-radius: 8px;
  background: ${props => props.variant === 'primary' ? '#4CAF50' : props.variant === 'danger' ? '#f44336' : '#2196F3'};
  color: white;
  font-size: 16px;
  font-weight: bold;
  cursor: pointer;
  transition: all 0.3s ease;
  margin-top: 10px;

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

const TransactionHistory = styled.div`
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-radius: 12px;
  padding: 25px;
  border: 1px solid rgba(255, 255, 255, 0.2);
`;

const TransactionTable = styled.table`
  width: 100%;
  border-collapse: collapse;
  margin-top: 15px;
  
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
  padding: 4px 12px;
  border-radius: 20px;
  font-size: 0.8em;
  font-weight: bold;
  background: ${props => 
    props.status === 'COMPLETED' ? '#4CAF50' : 
    props.status === 'PENDING' ? '#FF9800' : 
    props.status === 'FAILED' ? '#f44336' : '#666'
  };
  color: white;
`;

const CardElementContainer = styled.div`
  padding: 15px;
  border: 1px solid rgba(255, 255, 255, 0.3);
  border-radius: 8px;
  background: rgba(255, 255, 255, 0.1);
  margin-bottom: 20px;
`;

const StripePaymentForm = ({ amount, onSuccess }) => {
  const stripe = useStripe();
  const elements = useElements();
  const [processing, setProcessing] = useState(false);
  const [error, setError] = useState(null);

  const handleSubmit = async (event) => {
    event.preventDefault();
    
    if (!stripe || !elements) return;
    
    setProcessing(true);
    setError(null);
    
    const cardElement = elements.getElement(CardElement);
    
    // Simulate payment processing
    setTimeout(() => {
      setProcessing(false);
      onSuccess({
        id: 'txn_' + Date.now(),
        amount: amount,
        method: 'Credit Card',
        status: 'COMPLETED'
      });
    }, 2000);
  };

  return (
    <form onSubmit={handleSubmit}>
      <CardElementContainer>
        <CardElement
          options={{
            style: {
              base: {
                fontSize: '16px',
                color: '#fff',
                '::placeholder': {
                  color: 'rgba(255, 255, 255, 0.6)',
                },
              },
            },
          }}
        />
      </CardElementContainer>
      
      {error && <div style={{ color: '#f44336', marginBottom: '15px' }}>{error}</div>}
      
      <PaymentButton type="submit" disabled={!stripe || processing} variant="primary">
        {processing ? 'Processing...' : `Pay $${amount.toLocaleString()}`}
      </PaymentButton>
    </form>
  );
};

const Payment = () => {
  const [depositAmount, setDepositAmount] = useState('');
  const [withdrawAmount, setWithdrawAmount] = useState('');
  const [selectedMethod, setSelectedMethod] = useState('stripe');
  const [transactions, setTransactions] = useState([
    {
      id: 'txn_001',
      date: '2024-01-15',
      type: 'DEPOSIT',
      amount: 50000,
      method: 'Wire Transfer',
      status: 'COMPLETED'
    },
    {
      id: 'txn_002',
      date: '2024-01-20',
      type: 'DEPOSIT',
      amount: 25000,
      method: 'Credit Card',
      status: 'COMPLETED'
    },
    {
      id: 'txn_003',
      date: '2024-02-01',
      type: 'WITHDRAWAL',
      amount: 10000,
      method: 'Bank Transfer',
      status: 'PENDING'
    },
    {
      id: 'txn_004',
      date: '2024-02-10',
      type: 'DEPOSIT',
      amount: 15000,
      method: 'PayPal',
      status: 'COMPLETED'
    }
  ]);

  const [accountSummary] = useState({
    availableBalance: 1247850.75,
    pendingDeposits: 25000.00,
    pendingWithdrawals: 10000.00,
    totalDeposited: 90000.00,
    totalWithdrawn: 10000.00,
    netDeposits: 80000.00
  });

  const handleDeposit = (paymentResult) => {
    const newTransaction = {
      id: paymentResult.id,
      date: new Date().toISOString().split('T')[0],
      type: 'DEPOSIT',
      amount: parseFloat(depositAmount),
      method: paymentResult.method,
      status: paymentResult.status
    };
    
    setTransactions(prev => [newTransaction, ...prev]);
    setDepositAmount('');
    alert(`Deposit of $${parseFloat(depositAmount).toLocaleString()} initiated successfully!`);
  };

  const handleWithdrawal = () => {
    if (!withdrawAmount || parseFloat(withdrawAmount) <= 0) {
      alert('Please enter a valid withdrawal amount');
      return;
    }
    
    if (parseFloat(withdrawAmount) > accountSummary.availableBalance) {
      alert('Insufficient funds for withdrawal');
      return;
    }
    
    const newTransaction = {
      id: 'txn_' + Date.now(),
      date: new Date().toISOString().split('T')[0],
      type: 'WITHDRAWAL',
      amount: parseFloat(withdrawAmount),
      method: 'Bank Transfer',
      status: 'PENDING'
    };
    
    setTransactions(prev => [newTransaction, ...prev]);
    setWithdrawAmount('');
    alert(`Withdrawal of $${parseFloat(withdrawAmount).toLocaleString()} initiated successfully!`);
  };

  const formatCurrency = (value) => {
    return new Intl.NumberFormat('en-US', {
      style: 'currency',
      currency: 'USD'
    }).format(value);
  };

  return (
    <PaymentContainer>
      <PaymentHeader>💳 Account Management</PaymentHeader>
      
      {/* Account Summary */}
      <AccountSummary>
        <h3>Account Summary</h3>
        <SummaryGrid>
          <SummaryItem positive>
            <div className="label">Available Balance</div>
            <div className="value">{formatCurrency(accountSummary.availableBalance)}</div>
          </SummaryItem>
          <SummaryItem>
            <div className="label">Pending Deposits</div>
            <div className="value">{formatCurrency(accountSummary.pendingDeposits)}</div>
          </SummaryItem>
          <SummaryItem>
            <div className="label">Pending Withdrawals</div>
            <div className="value">{formatCurrency(accountSummary.pendingWithdrawals)}</div>
          </SummaryItem>
          <SummaryItem positive>
            <div className="label">Total Deposited</div>
            <div className="value">{formatCurrency(accountSummary.totalDeposited)}</div>
          </SummaryItem>
          <SummaryItem>
            <div className="label">Total Withdrawn</div>
            <div className="value">{formatCurrency(accountSummary.totalWithdrawn)}</div>
          </SummaryItem>
          <SummaryItem positive>
            <div className="label">Net Deposits</div>
            <div className="value">{formatCurrency(accountSummary.netDeposits)}</div>
          </SummaryItem>
        </SummaryGrid>
      </AccountSummary>

      {/* Payment Methods */}
      <PaymentGrid>
        {/* Deposit */}
        <PaymentCard>
          <h3>💰 Deposit Funds</h3>
          
          <FormGroup>
            <label>Amount</label>
            <input
              type="number"
              placeholder="Enter deposit amount"
              value={depositAmount}
              onChange={(e) => setDepositAmount(e.target.value)}
              min="100"
              step="100"
            />
          </FormGroup>
          
          <FormGroup>
            <label>Payment Method</label>
            <select value={selectedMethod} onChange={(e) => setSelectedMethod(e.target.value)}>
              <option value="stripe">Credit/Debit Card</option>
              <option value="paypal">PayPal</option>
              <option value="wire">Wire Transfer</option>
            </select>
          </FormGroup>

          {selectedMethod === 'stripe' && depositAmount && (
            <StripePaymentForm 
              amount={parseFloat(depositAmount)} 
              onSuccess={handleDeposit}
            />
          )}

          {selectedMethod === 'paypal' && depositAmount && (
            <PayPalScriptProvider options={{ "client-id": "demo-client-id" }}>
              <PayPalButtons
                style={{ layout: "vertical" }}
                createOrder={(data, actions) => {
                  return actions.order.create({
                    purchase_units: [{
                      amount: {
                        value: depositAmount
                      }
                    }]
                  });
                }}
                onApprove={(data, actions) => {
                  return actions.order.capture().then(() => {
                    handleDeposit({
                      id: data.orderID,
                      amount: parseFloat(depositAmount),
                      method: 'PayPal',
                      status: 'COMPLETED'
                    });
                  });
                }}
              />
            </PayPalScriptProvider>
          )}

          {selectedMethod === 'wire' && depositAmount && (
            <div>
              <div style={{ background: 'rgba(255,255,255,0.1)', padding: '15px', borderRadius: '8px', marginBottom: '15px' }}>
                <strong>Wire Transfer Instructions:</strong><br />
                Bank: HedgeFund Bank<br />
                Account: 123-456-7890<br />
                Routing: 021000021<br />
                Reference: Your Account ID
              </div>
              <PaymentButton variant="primary" onClick={() => handleDeposit({
                id: 'wire_' + Date.now(),
                amount: parseFloat(depositAmount),
                method: 'Wire Transfer',
                status: 'PENDING'
              })}>
                Initiate Wire Transfer
              </PaymentButton>
            </div>
          )}
        </PaymentCard>

        {/* Withdrawal */}
        <PaymentCard>
          <h3>💸 Withdraw Funds</h3>
          
          <FormGroup>
            <label>Amount</label>
            <input
              type="number"
              placeholder="Enter withdrawal amount"
              value={withdrawAmount}
              onChange={(e) => setWithdrawAmount(e.target.value)}
              min="100"
              step="100"
              max={accountSummary.availableBalance}
            />
          </FormGroup>
          
          <FormGroup>
            <label>Withdrawal Method</label>
            <select>
              <option value="bank">Bank Transfer (ACH)</option>
              <option value="wire">Wire Transfer</option>
              <option value="check">Check</option>
            </select>
          </FormGroup>

          <FormGroup>
            <label>Bank Account</label>
            <select>
              <option value="primary">Primary Account (...4567)</option>
              <option value="savings">Savings Account (...8901)</option>
            </select>
          </FormGroup>

          <div style={{ background: 'rgba(255,255,255,0.1)', padding: '15px', borderRadius: '8px', marginBottom: '15px', fontSize: '0.9em' }}>
            <strong>Processing Times:</strong><br />
            • ACH Transfer: 1-3 business days<br />
            • Wire Transfer: Same day<br />
            • Check: 5-7 business days
          </div>

          <PaymentButton variant="danger" onClick={handleWithdrawal}>
            Request Withdrawal
          </PaymentButton>
        </PaymentCard>
      </PaymentGrid>

      {/* Transaction History */}
      <TransactionHistory>
        <h3>Transaction History</h3>
        <TransactionTable>
          <thead>
            <tr>
              <th>Date</th>
              <th>Transaction ID</th>
              <th>Type</th>
              <th>Amount</th>
              <th>Method</th>
              <th>Status</th>
            </tr>
          </thead>
          <tbody>
            {transactions.map((transaction) => (
              <tr key={transaction.id}>
                <td>{transaction.date}</td>
                <td>{transaction.id}</td>
                <td>
                  <span style={{ 
                    color: transaction.type === 'DEPOSIT' ? '#4CAF50' : '#FF9800',
                    fontWeight: 'bold'
                  }}>
                    {transaction.type}
                  </span>
                </td>
                <td style={{ 
                  color: transaction.type === 'DEPOSIT' ? '#4CAF50' : '#f44336'
                }}>
                  {transaction.type === 'DEPOSIT' ? '+' : '-'}{formatCurrency(transaction.amount)}
                </td>
                <td>{transaction.method}</td>
                <td>
                  <StatusBadge status={transaction.status}>
                    {transaction.status}
                  </StatusBadge>
                </td>
              </tr>
            ))}
          </tbody>
        </TransactionTable>
      </TransactionHistory>
    </PaymentContainer>
  );
};

export default Payment;