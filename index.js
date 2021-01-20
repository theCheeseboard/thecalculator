// if (!process.env.BOT_TOKEN || !process.env.BOT_USER_ID) {
//     console.log("ERROR: Environment variables not set.");
//     console.log("Set BOT_TOKEN to your bot's token and BOT_USER_ID to your bot's user ID.");
//     return;
// }

const port = process.env.PORT || 4000;

const express = require('express');
const interactions = require('discord-slash-commands-client');

let app = express();
app.use(express.json({
    limit: "10mb"
}));

const client = new interactions.Client(process.env.BOT_TOKEN, process.env.BOT_USER_ID);
client.createCommand({
    name: "calc",
    description: "Evaluate a mathematical expression"
}, "336487228228370432").then((command) => {
    
}).catch(reason => {
    console.log("Could not create command");
    console.log(reason);
})

app.post("/", (req, res) => {
    //Validate signature headers

    if (req.body.type === 1) {
        //PING message
        res.send({
            type: 1
        });
    }
});

app.listen(port, () => {
    console.log(`Server ready on port ${port}!`);
});