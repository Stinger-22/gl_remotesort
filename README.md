# Remotesort

App was made on Arch Linux 6.8.7

## Task

Client / Server application to retrieve list of files in sorted order.
- The server has to sort files in his folder and sub-folders
- The client will request sorting by name/type/date
- The server has to reply with success or failure
- Success replay has to include a list of files and number of files in his folder and sub-folders.

## Usage

- Use make from project directory to compile client and server: `make`
- Start server with the command: `server [port]`. If server hasn't started or port is already used you'll get logs in the console.
- You can use client with the command: `client [server-hostname] [port]`. You'll be asked to type directory path on the server and sorting type. If request succeeds list of filenames with last modify time will be printed in console.
