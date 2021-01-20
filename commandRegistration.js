const commandClient = require('discord-slash-commands-client');

module.exports = async () => {
    let client = new commandClient.Client(process.env.BOT_TOKEN, process.env.BOT_USER_ID);
    let command = await client.createCommand({
        name: "calc",
        description: "Evaluate a mathematical expression",
        options: [
            {
                name: "expression",
                description: "Expression to evaluate",
                default: false,
                required: true,
                type: 3
            },
            {
                name: "trigunit",
                description: "Trigonometric unit to use",
                default: false,
                required: false,
                type: 3,
                choices: [
                    {
                        name: "degrees",
                        value: "degrees"
                    },
                    {
                        name: "radians",
                        value: "radians"
                    },
                    {
                        name: "gradians",
                        value: "gradians"
                    }
                ]
            }
        ]
    }, "336487228228370432");
}
