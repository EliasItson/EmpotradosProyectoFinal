let enfoque = false;

function actualizarEstado() {
  fetch("/api/getStatus").then(r=>r.json()).then(d=>{
    document.getElementById("rfid").innerText="RFID: "+d.rfidUID;
    document.getElementById("distancia").innerText="Distancia: "+d.distancia.toFixed(2)+" cm";
    document.getElementById("plumaEntrada").innerText="Pluma Entrada: "+(d.plumaEntrada?"Abierta":"Cerrada");
    document.getElementById("plumaSalida").innerText="Pluma Salida: "+(d.plumaSalida?"Abierta":"Cerrada");
    document.getElementById("cajon1").innerText="Cajón 1: "+(d.cajon1?"Ocupado":"Libre");
    document.getElementById("cajon2").innerText="Cajón 2: "+(d.cajon2?"Ocupado":"Libre");
    document.getElementById("entry1").innerText="Entrada 1: "+ (d.entryTime1?d.entryTime1:"--");
    document.getElementById("exit1").innerText="Salida 1: "+ (d.exitTime1?d.exitTime1:"--");
    document.getElementById("entry2").innerText="Entrada 2: "+ (d.entryTime2?d.entryTime2:"--");
    document.getElementById("exit2").innerText="Salida 2: "+ (d.exitTime2?d.exitTime2:"--");
  }).catch(e=>console.error("Error:",e));
}

function actualizarParametros() {
  if(!enfoque) {
    fetch("/api/getParams").then(r=>r.json()).then(d=>{
      document.getElementById("delaySalida").value=d.SALIDA_DELAY_MS;
      document.getElementById("timeoutUltrasonico").value=d.ULTRASONIC_TIMEOUT_MS;
    }).catch(e=>console.error("Error:",e));
  }
}

function guardarParametros() {
  let p={SALIDA_DELAY_MS:parseInt(document.getElementById("delaySalida").value),ULTRASONIC_TIMEOUT_MS:parseInt(document.getElementById("timeoutUltrasonico").value)};
  fetch("/api/setParams",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify(p)}).then(()=>{alert("Guardado");enfoque=false;actualizarParametros();}).catch(e=>console.error("Error:",e));
}

document.getElementById("delaySalida").addEventListener("focus",()=>{enfoque=true;});
document.getElementById("delaySalida").addEventListener("blur",()=>{enfoque=false;});
document.getElementById("timeoutUltrasonico").addEventListener("focus",()=>{enfoque=true;});
document.getElementById("timeoutUltrasonico").addEventListener("blur",()=>{enfoque=false;});
setInterval(()=>{actualizarEstado();actualizarParametros();},1000);
actualizarEstado();actualizarParametros();
