const child = require('child_process');
const interactionsClient = require('discord-interactions');

module.exports = (body, options) => {
    let data = body.data;

    return new Promise((res, rej) => {
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
                if (error.code === 1) {
                    res({
                        content: `An error occurred:\n${stdout.trim()}`
                    })
                } else {
                    res({
                        content: `Hmm, we couldn't start the theCalculator engine. Let Victor know ASAP!`,
                    });
                }
                return;
            }
    
    
            res({
                content: `The answer is ${stdout.trim()}!`,
            });
        });
    });

}

