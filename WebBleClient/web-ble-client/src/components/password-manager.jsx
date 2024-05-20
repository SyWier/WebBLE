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

unicom.addCallback(() => console.log("Unknown data received."));

function PasswordManager() {
  const [textbox, setTextbox] = useState({text: 'Request data to display text...', id: undefined});

  // Function to request data and modify textbox text
  async function requestValue(command) {
    try {
      setTextbox({ text: 'In progress...', id: undefined });
      let packet = await unicom.requestValue(new Uint8Array([command]));
      setTextbox({ text: packet.data, id: packet?.extraData?.id });
    } catch(e) {
      setTextbox({ text: e.toString(), id: undefined });
      console.log(e);
    }
  }

  async function requestString(command) {
    try {
      setTextbox({ text: 'In progress...', id: undefined });
      let packet = await unicom.requestString(new Uint8Array([command]));
      setTextbox({ text: packet.data, id: packet?.extraData?.id });
    } catch(e) {
      setTextbox({ text: e.toString(), id: undefined });
      console.log(e);
    }
  }

  async function requestJSON(command) {
    try {
      setTextbox({ text: 'In progress...', id: undefined });
      let packet = await unicom.requestJSON(new Uint8Array([command]));
    let jsonString = JSON.stringify(packet.data, null, 2);
    setTextbox({ text: jsonString, id: packet?.extraData?.id });
    } catch(e) {
      setTextbox({ text: e.toString(), id: undefined });
      console.log(e);
    }
  }

  // Textbox element to display answer from the server
  function RequestedText(props) {
    let idText = undefined;
    if(props.textbox.id) {
      idText = <Typography>Message ID: {props.textbox.id}</Typography>;
    }

    return(
      <Box sx={{width: '75%'}}>
        <h1>Requested Text</h1>
        <Typography style={{overflowWrap: 'break-word'}}>
          {props.textbox.text}
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
          <Button onClick={() => requestValue(1)} variant="contained">Value</Button>
          <Button onClick={() => requestString(2)} variant="contained">String</Button>
          <Button onClick={() => requestJSON(3)} variant="contained">JSON</Button>
        </Stack>
        <h1>Send</h1>
        <Stack direction="row" spacing={1}>
          <Button onClick={() => unicom.sendValue(valueToSend, valueExtraData)} variant="contained" >Value</Button>
          <Button onClick={() => unicom.sendString(stringToSend)} variant="contained">String</Button>
          <Button onClick={() => unicom.sendJSON(objectToSend)} variant="contained">JSON</Button>
        </Stack>
      </Box>
      <RequestedText textbox={textbox}/>
    </Stack>
  );
};

export default PasswordManager;

export { webble };
