import React from 'react';
import ReactDOM from 'react-dom/client';
import './styles/index.css';
import { Container, Box } from '@mui/material';
import PasswordManager from './components/password-manager';
import Header from './components/header';
import Footer from './components/footer';


const root = ReactDOM.createRoot(document.getElementById('root'));
root.render(
  <React.StrictMode>
    <Box sx={{minHeight: '100vh'}}>
      <Header />
      <Container sx={{height: '82vh'}}> {/* Why I can't make it 100%? :'( */}
        <PasswordManager />
      </Container>
      <Footer />
    </Box>
  </React.StrictMode>
);
