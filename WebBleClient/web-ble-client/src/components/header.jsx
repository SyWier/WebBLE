import React, { useState, useEffect } from 'react';
import { AppBar, Container, Toolbar, Typography, Button } from '@mui/material';
import BluetoothIcon from '@mui/icons-material/Bluetooth';
import { webble } from './password-manager';

function ConnectButton() {
    const [isConnected, updateConnection] = useState(false);

    // Update state
    function updateState(isConnected) {
      updateConnection(isConnected);
    }

    // Add callback only once
    useEffect(() => {
      webble.addCallback(updateState);
    }, []);

    function handleClick() {
        if(isConnected) {
            webble.disconnectDevice()
        } else {
            webble.connectToDevice()
        }
    }

    let button;
    if(isConnected) {
      button = <Button onClick={() => handleClick()} variant="contained" color="error">Disconnect</Button>
    } else {
        button = <Button onClick={() => handleClick()} variant="contained" color="success" sx={{mr: 2}}>Connect</Button>
    }

    return(<>{button}</>);
}

function Header() {
  return (
    <AppBar position="static">
      <Container maxWidth="xl">
        <Toolbar disableGutters>
          <BluetoothIcon fontSize="large" sx={{ mr: 1, display: { xs: 'none', md: 'flex'}}}/>
          <Typography
            variant="h4" noWrap component="a"
            sx={{ mr: 2, fontWeight: 700, display: { xs: 'none', md: 'flex' }}}
          > WebBLE
          </Typography>
          <ConnectButton />
        </Toolbar>
      </Container>
    </AppBar>
  );
}

export default Header;
