import React, { useState } from "react";
import { Box, Button, Stack, Typography } from "@mui/material";
import WebBLE from "../libs/webble";
import UniCom from "../libs/unicom";

/* Example project to showcase the use of the
** webble and unicom libraries */

/* Create WebBLE and UniCom service to
** communicate with the BLE server */
var webble = new WebBLE;
var unicom = new UniCom(webble);
webble.addService(unicom);

function PasswordManager() {
  const[text, setText] = useState('Request data to display text...');
  const[id, setID] = useState(undefined)

  // Function to request data and modify textbox text
  function requestData(command) {
    try {
      console.log("Request value by command:", command);
      unicom.sendValue(new Uint8Array([command]));
      setText("In progress...");
    } catch(error) {
        console.error("Error requesting data: ", error);
    }
  }

  // The callback function to receive and process data from
  // the unicom library
  function requestCallback(packet) {
    console.log(packet?.extraData?.id);
    setID(packet?.extraData?.id);
    switch(packet.dataType) {
      case unicom.dataType.value:
        setText(packet.data);
        return;
      case unicom.dataType.string:
        setText(new TextDecoder().decode(packet.data));
        return;
      case unicom.dataType.json:
        let jsonString = new TextDecoder().decode(packet.data);
        let json = JSON.parse(jsonString);
        let jsonPretty = JSON.stringify(json, null, 2);
        setText(jsonString);
        console.log(json);
        return;
      default:
        console.log("Unkown data type received");
        console.log(unicom.bufferToHex(packet.data));
    }
  }
  unicom.addCallback(requestCallback);

  // Textbox element to display answer from the server
  function RequestedText(props) {
    let idText = undefined;
    if(props.id) {
      idText = <Typography>Message ID: {props.id}</Typography>;
    }

    return(
      <Box sx={{width: '75%'}}>
        <h1>Requested Text</h1>
        <Typography style={{overflowWrap: 'break-word'}}>
          {props.text}
        </Typography>
        {idText}
      </Box>
    );
  }

  // Value to send (with extra information)
  const valueToSend = new Uint8Array([10]);
  const valueExtraData = {
    flags : unicom.flags.id_flag,
    data : { id : 123 },
  }

  // String to send
  const stringToSend = "Hello BLE!";

  // Object to send
  const objectToSend = {
    username : 'Sany9',
    email : 'sanyika02@gmail.com',
    password : 'superS3cr3t'
  };

  return (
    <Stack direction="row">
      <Box sx={{width: '25%'}}>
        <h1>Request</h1>
        <Stack direction="row" spacing={1}>
          <Button onClick={() => requestData(1)} variant="contained">Value</Button>
          <Button onClick={() => requestData(2)} variant="contained">String</Button>
          <Button onClick={() => requestData(3)} variant="contained">JSON</Button>
        </Stack>
        <h1>Send</h1>
        <Stack direction="row" spacing={1}>
          <Button onClick={() => unicom.sendValue(valueToSend, valueExtraData)} variant="contained" >Value</Button>
          <Button onClick={() => unicom.sendString(stringToSend)} variant="contained">String</Button>
          <Button onClick={() => unicom.sendJSON(objectToSend)} variant="contained">JSON</Button>
        </Stack>
      </Box>
      <RequestedText text={text} id={id}/>
    </Stack>
  );
};

export default PasswordManager;

export { webble };
