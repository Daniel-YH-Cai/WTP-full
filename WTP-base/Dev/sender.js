const dgram=require('dgram')
const client=dgram.createSocket('udp4')

setTimeout(()=>{
    console.log("Sending started")
    client.send("DemoData1",8888,'localhost',()=>{
        console.log("First send")
        client.send("DemoData2",8888,'localhost',()=>{
            console.log("Second send")
            client.send("DemoData3",8888,'localhost',()=>{

            })
        })
    })
},2000);


