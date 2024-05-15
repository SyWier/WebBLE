import React from 'react';
import { AppBar, Container, Toolbar, Typography, Button } from '@mui/material';
import BluetoothIcon from '@mui/icons-material/Bluetooth';
import { webble } from './password-manager';

class ConnectButton extends React.Component {
    constructor(props) {
    super(props);
        this.state = { isConnected: false };
    }

    handleClick() {
        if(this.state.isConnected) {
            webble.disconnectDevice()
        } else {
            webble.connectToDevice()
        }

        this.setState(prevState => ({
            isConnected: !prevState.isConnected
        }));
    }

    render() {
        let button;
        if(this.state.isConnected) {
            button = <Button onClick={() => this.handleClick()} variant="contained" color="error">Disconnect</Button>
        } else {
            button = <Button onClick={() => this.handleClick()} variant="contained" color="success" sx={{mr: 2}}>Connect</Button>
        }

        return(<div>{button}</div>);
    }
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
