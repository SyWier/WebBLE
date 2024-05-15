import React from "react";
import { Box, Button, Stack, Typography } from "@mui/material";
import WebBLE from "../libs/webble";
import UniCom from "../libs/unicom";

var webble = new WebBLE;
var unicom = new UniCom(webble);
webble.addService(unicom);

function sendSomeValue() {
  let extraData = {
    flags : unicom.flags.id_flag,
    data : { id : 123 },
  }
  unicom.sendValue(new Uint8Array([10]), extraData);
}

function sendSomeString() {
  let string = "Hello BLE!";
  unicom.sendString(string);
}

function sendSomeJSON() {
  let object = {
    username : 'Sany9',
    email : 'sanyika02@gmail.com',
    password : 'superS3cr3t'
  };
  unicom.sendJSON(object)
}

function RequestedText(props) {
  return(
    <Box sx={{width: '75%'}}>
      <h1>Requested Text</h1>
      <Typography style={{overflowWrap: 'break-word'}}>
        {props.text}
      </Typography>
    </Box>
  );
}

class PasswordManager extends React.Component {
  constructor(props) {
    super(props);
    this.state = {text: "Empty"};
    unicom.addCallback(this.requestCallback.bind(this));
  }

  requestData(command) {
    try {
      console.log("Request value by command:", command);
      unicom.sendValue(new Uint8Array([command]));
      this.setState({text: "In progress..."});
    } catch(error) {
        console.error("Error requesting data: ", error);
    }
  }

  requestCallback(packet) {
    let val;
    switch(packet.dataType) {
      case unicom.dataType.value:
        val = packet.data;
        break;
      case unicom.dataType.string:
        val = new TextDecoder().decode(packet.data);
        break;
      case unicom.dataType.json:
        let jsonString = new TextDecoder().decode(packet.data);
        let json = JSON.parse(jsonString);
        let jsonPretty = JSON.stringify(json, null, 2);
        val = jsonPretty;
        console.log(json);
        break;
      default:
        console.log("Unkown data type received");
        console.log(unicom.bufferToHex(packet.data));
    }

    this.setState({text: val});
  }

  render() {
    return (
      <Stack direction="row">
        <Box sx={{width: '25%'}}>
          <h1>Request</h1>
          <Stack direction="row" spacing={1}>
            <Button onClick={() => this.requestData(1)} variant="contained">Value</Button>
            <Button onClick={() => this.requestData(2)} variant="contained">String</Button>
            <Button onClick={() => this.requestData(3)} variant="contained">JSON</Button>
          </Stack>
          <h1>Send</h1>
          <Stack direction="row" spacing={1}>
            <Button onClick={() => sendSomeValue()} variant="contained" >Value</Button>
            <Button onClick={() => sendSomeString()} variant="contained">String</Button>
            <Button onClick={() => sendSomeJSON()} variant="contained">JSON</Button>
          </Stack>
        </Box>
        <RequestedText text={this.state.text}/>
      </Stack>
    );
  };
};

export default PasswordManager;

export { webble };
