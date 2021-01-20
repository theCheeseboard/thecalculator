const child = require('child_process');

module.exports = (body, options) => {
    return new Promise(res => {
        let args = [
            "-e",
            options.expression,
            "-c"
        ];

        if (options.trigunit) {
            args.push("-t");
            args.push(options.trigunit);
        }

        child.execFile("thecalculator", args, {
            env: {
                ...process.env,
                "QT_QPA_PLATFORM": "offscreen"
            }
        }, (error, stdout, stderr) => {
            if (error) {
                if (error.code === 1) {
                    //Parse stdout
                    let interestingLines = stdout.split("\n").filter(line => line !== "");
                    while (interestingLines.length > 3) interestingLines.shift();

                    let locationInformation = interestingLines[1];
                    let start = interestingLines[2].indexOf("^");
                    let end = interestingLines[2].lastIndexOf("^") + 1;
                    locationInformation = [locationInformation.slice(0, start), "**", locationInformation.slice(start, end), "**", locationInformation.slice(end)].join('');

                    res({
                        embeds: [{
                            title: `Evaluate: ${options.expression}`,
                            fields: [
                                {
                                    name: "Error",
                                    value: interestingLines[0]
                                },
                                {
                                    name: "Location",
                                    value: locationInformation
                                }
                            ],
                            color: 0x640000
                        }]
                    })
                } else {
                    res({
                        content: `Hmm, we couldn't start the theCalculator engine. Let Victor know ASAP!`,
                    });
                }
                return;
            }

            let lines = stdout.split("\n").filter(line => line !== "");
            if (lines.length === 1) {
                res({
                    embeds: [{
                        title: `Evaluate: ${options.expression}`,
                        description: `The answer is **${lines[0]}**`,
                        color: 0x0064FF
                    }]
                });
            } else {
                //This is a multi-calc
                res({
                    content: `Standard output follows:\n${stdout.trim()}`,
                });
            }
        });
    });
}

