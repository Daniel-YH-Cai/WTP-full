//nodejs server for testing
const dgram=require('dgram')
const server=dgram.createSocket('udp4')

server.on('listening',()=>{
    console.log("Started!")
})

server.on('message',(msg,rinfo)=>{
    console.log('Message: ')
    console.log(msg.toString())
})

server.bind(8888)
