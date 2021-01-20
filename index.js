// if (!process.env.BOT_TOKEN || !process.env.BOT_USER_ID || !process.env.BOT_PUBLIC_KEY) {
//     console.log("ERROR: Environment variables not set.");
//     console.log("Set BOT_TOKEN to your bot's token, BOT_USER_ID to your bot's user ID and BOT_PUBLIC_KEY to your bot's public key.");
//     return;
// }

const server = require('./server');
const registration = require('./commandRegistration');