const express = require('express');
let app = express();

app.post("/", (res, rej) => {
    //Validate signature headers
    
});

app.listen(6000, () => {
    console.log("Server ready!");
});