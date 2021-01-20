// if (!process.env.BOT_TOKEN || !process.env.BOT_USER_ID || !process.env.BOT_PUBLIC_KEY) {
//     console.log("ERROR: Environment variables not set.");
//     console.log("Set BOT_TOKEN to your bot's token, BOT_USER_ID to your bot's user ID and BOT_PUBLIC_KEY to your bot's public key.");
//     return;
// }

const port = process.env.PORT || 4000;

const express = require('express');
const commandClient = require('discord-slash-commands-client');
const interactionsClient = require('discord-interactions');

let app = express();

const client = new commandClient.Client(process.env.BOT_TOKEN, process.env.BOT_USER_ID);
client.createCommand({
    name: "calc",
    description: "Evaluate a mathematical expression"
}, "336487228228370432").then((command) => {
    
}).catch(reason => {
    console.log("Could not create command");
    console.log(reason);
})

app.post("/", interactionsClient.verifyKeyMiddleware(process.env.BOT_PUBLIC_KEY), (req, res) => {
    //Validate signature headers

    if (req.body.type === interactionsClient.InteractionType.COMMAND) {
        res.send({
          type: InteractionResponseType.CHANNEL_MESSAGE_WITH_SOURCE,
          data: {
            content: 'Hello world',
          },
        });
    }
});

app.listen(port, () => {
    console.log(`Server ready on port ${port}!`);
});