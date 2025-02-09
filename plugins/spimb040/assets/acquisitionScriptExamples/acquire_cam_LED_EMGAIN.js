
params=new Array(0,1,5);//,10,15,20,30,40,50, 0);
emgain=new Array(0,1,5);//,10,50,100,200,300);
delay=2000;
frames=20;

for (var i=0; i<params.length; i++) {
    tools.logText("setting LED to " + params[i]+"\n");
    instrument.setBrightfieldIntensity(0, 0, params[i]);
    tools.sleepMS(  delay);
    tools.logText("running acquisition at " + params[i]+"mA\n");
    instrument.setAcqDescCurrentSample("LED_"+params[i]+"mA_");
    for (var j=0; j<params.length; j++) {
        acquisition.setPreviewParam("cam_andor/emgain_enabled", true);
        acquisition.setPreviewParam("cam_andor/emgain", emgain[j]);
        acquisition.acquirePreviewFramesWithParams(0, frames, "%prefix%_emgain"+emgain[j]+"_%counter%");
    }
//    acquisition.doAcquisition();
    acquisition.clearPreviewParams();
}
