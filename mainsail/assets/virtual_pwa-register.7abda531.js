import{b$ as v}from"./index.2d74e0f6.js";function p(d={}){const{immediate:l=!1,onNeedRefresh:g,onOfflineReady:i,onRegistered:r,onRegisteredSW:o,onRegisterError:s}=d;let t,n;const c=async(a=!0)=>{await n};async function f(){if("serviceWorker"in navigator){const{Workbox:a}=await v(()=>import("./workbox-window.prod.es5.e0cc53cf.js"),[]);t=new a("/sw.ts",{scope:"/",type:"classic"}),t.addEventListener("activated",e=>{(e.isUpdate||e.isExternal)&&window.location.reload()}),t.addEventListener("installed",e=>{e.isUpdate||i==null||i()}),t.register({immediate:l}).then(e=>{o?o("/sw.ts",e):r==null||r(e)}).catch(e=>{s==null||s(e)})}}return n=f(),c}export{p as registerSW};
