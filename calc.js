const child = require('child_process');
const interactionsClient = require('discord-interactions');

module.exports = (body, options, res) => {
    let data = body.data;

    child.execFile("thecalculator", [
        "-e",
        options.expression,
        "-c"
    ], {
        env: {
            ...process.env,
            "QT_QPA_PLATFORM": "offscreen"
        }
    }, (error, stdout, stderr) => {
        if (error) {
            res.send({
                type: interactionsClient.InteractionResponseType.CHANNEL_MESSAGE_WITH_SOURCE,
                data: {
                  content: "Hmm, we couldn't start the theCalculator engine. Let Victor know ASAP!",
                },
            });
            return;
        }


        res.send({
            type: interactionsClient.InteractionResponseType.CHANNEL_MESSAGE_WITH_SOURCE,
            data: {
              content: `The answer is ${stdout}!`,
            },
        });
    });

}