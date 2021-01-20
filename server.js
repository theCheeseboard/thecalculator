const port = process.env.PORT || 4000;

const express = require('express');
const interactionsClient = require('discord-interactions');

let commands = {
    "calc": require('./calc')
};

let app = express();
app.post("/", interactionsClient.verifyKeyMiddleware(process.env.BOT_PUBLIC_KEY), (req, res) => {
    try {
        if (req.body.type === interactionsClient.InteractionType.COMMAND) {
            let data = req.body.data;
            let options = {};

            for (let option of data.options) {
                options[option.name] = option.value;
            }

            res.send({
                type: interactionsClient.InteractionResponseType.CHANNEL_MESSAGE_WITH_SOURCE,
                data: await (commands[data.name])(req.body, options)
            });
            return;
        }
    } catch (err) {
        res.send(500);
    }
});

app.listen(port, () => {
    console.log(`Server ready on port ${port}!`);
});

module.exports = app;