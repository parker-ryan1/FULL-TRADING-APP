import React from 'react';
import { Link, useLocation } from 'react-router-dom';
import styled from 'styled-components';

const HeaderContainer = styled.header`
  background: rgba(0, 0, 0, 0.2);
  backdrop-filter: blur(10px);
  padding: 1rem 2rem;
  border-bottom: 1px solid rgba(255, 255, 255, 0.1);
`;

const Nav = styled.nav`
  display: flex;
  justify-content: space-between;
  align-items: center;
  max-width: 1400px;
  margin: 0 auto;
`;

const Logo = styled.h1`
  color: #fff;
  margin: 0;
  font-size: 1.5rem;
  font-weight: bold;
`;

const NavLinks = styled.div`
  display: flex;
  gap: 2rem;
`;

const NavLink = styled(Link)`
  color: ${props => props.active ? '#4CAF50' : 'rgba(255, 255, 255, 0.8)'};
  text-decoration: none;
  font-weight: 500;
  transition: color 0.3s ease;
  
  &:hover {
    color: #4CAF50;
  }
`;

const UserInfo = styled.div`
  display: flex;
  align-items: center;
  gap: 1rem;
  color: rgba(255, 255, 255, 0.8);
`;

function Header() {
  const location = useLocation();

  return (
    <HeaderContainer>
      <Nav>
        <Logo>HedgeFund Pro</Logo>
        
        <NavLinks>
          <NavLink to="/" active={location.pathname === '/' ? 1 : 0}>
            Dashboard
          </NavLink>
          <NavLink to="/trading" active={location.pathname === '/trading' ? 1 : 0}>
            Trading
          </NavLink>
          <NavLink to="/options" active={location.pathname === '/options' ? 1 : 0}>
            Options
          </NavLink>
          <NavLink to="/algo" active={location.pathname === '/algo' ? 1 : 0}>
            Algo Trading
          </NavLink>
          <NavLink to="/backtesting" active={location.pathname === '/backtesting' ? 1 : 0}>
            Backtesting
          </NavLink>
          <NavLink to="/portfolio" active={location.pathname === '/portfolio' ? 1 : 0}>
            Portfolio
          </NavLink>
          <NavLink to="/analytics" active={location.pathname === '/analytics' ? 1 : 0}>
            Analytics
          </NavLink>
          <NavLink to="/risk" active={location.pathname === '/risk' ? 1 : 0}>
            Risk
          </NavLink>
          <NavLink to="/payment" active={location.pathname === '/payment' ? 1 : 0}>
            Payment
          </NavLink>
        </NavLinks>
        
        <UserInfo>
          <span>Balance: $1,250,000</span>
          <span>|</span>
          <span>John Trader</span>
        </UserInfo>
      </Nav>
    </HeaderContainer>
  );
}

export default Header;