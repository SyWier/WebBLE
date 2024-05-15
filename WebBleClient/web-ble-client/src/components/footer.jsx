import React from 'react';
import { Box, Typography } from '@mui/material';

function Footer() {
  return(
    <Box
      sx={{
        minHeight: '3em',
        bgcolor: 'primary.main',
        color: 'white',
        display: 'flex',
        justifyContent: 'center',
        alignItems: 'center',
      }}
    >
      <Typography
        variant="h6" noWrap component="a"
        sx={{ fontWeight: 700, display: { xs: 'none', md: 'flex' }}}
      >
          Készítette: Sinkó Dániel (KUR6F9)
      </Typography>
    </Box>
  );
}

export default Footer;
