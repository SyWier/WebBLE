import React from "react";
import Button from '@mui/material/Button';

class Clock extends React.Component {
    constructor(props) {
        super(props);
        this.state = {date: new Date()};
    }

    componentDidMount() {
        this.timerID = setInterval(
            () => this.tick(),
            1000
        );
    }

    componentWillUnmount() {
        clearInterval(this.timerID);
    }

    tick() {
        this.setState({
            date: new Date()
        });
    }

    render() {
        return (
        <div>
            <h1>Hello, world!</h1>
            <h2>It is {this.state.date.toLocaleTimeString()}.</h2>
        </div>
        );
    }
}
  
class Toggle extends React.Component {
    constructor(props) {
        super(props);
        this.state = {isToggleOn: true};

        // This binding is necessary to make `this` work in the callback
        this.handleClick = this.handleClick.bind(this);
    }

    handleClick() {
        this.setState(prevState => ({
            isToggleOn: !prevState.isToggleOn
        }));
    }

    buttonText() {
        return this.state.isToggleOn ? 'ON' : 'OFF';
    }

    render() {
        return (
            <Button onClick={this.handleClick} variant="contained">
                {this.buttonText()}
            </Button>
        );
    }
}

class LoginControl extends React.Component {
    constructor(props) {
        super(props);
        this.handleLoginClick = this.handleLoginClick.bind(this);
        this.handleLogoutClick = this.handleLogoutClick.bind(this);
        this.state = {isLoggedIn: false};
    }

    handleLoginClick() {
        this.setState({isLoggedIn: true});
    }

    handleLogoutClick() {
        this.setState({isLoggedIn: false});
    }

    UserGreeting() {
        return <h1>Welcome back!</h1>;
    }
      
    GuestGreeting() {
        return <h1>Please sign up.</h1>;
    }
    
    Greeting(isLoggedIn) {
        if (isLoggedIn) {
            return this.UserGreeting();
        }
        return this.GuestGreeting();
    }
    
    LoginButton(func) {
        return (
            <Button onClick={func} variant="contained">
                Login
            </Button>
        );
    }
    
    LogoutButton(func) {
        return (
            <Button onClick={func} variant="contained">
                Logout
            </Button>
        );
    }

    render() {
        const isLoggedIn = this.state.isLoggedIn;
        let button;
        if (isLoggedIn) {
            button = this.LogoutButton(this.handleLogoutClick);
        } else {
            button = this.LoginButton(this.handleLoginClick);
        }
        let text = this.Greeting(isLoggedIn);

        return (
        <div>
            {text}
            {button}
        </div>
        );
    }
}

export {
    Clock,
    Toggle,
    LoginControl,
}
