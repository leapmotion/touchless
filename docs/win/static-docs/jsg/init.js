var tutorials = [];

// Sets up the controller
var LeapController = new Leap.Controller({enableGestures:true});

// Defines the controller's frame function
LeapController.on( 'frame' , function(data){
 
  this.currentFrame = data;

  if( ! this.oldFrame ){
    this.oldFrame = this.currentFrame;
    // Starts the examples if frames roll before device is connected
    for( var i = 0; i < tutorials.length; i ++){
      if( tutorials[i].active ){
        tutorials[i].deviceConnect();
      }
    }
  }

  for( var i = 0; i < tutorials.length; i ++){
    
    if( tutorials[i].active ){
      tutorials[i].draw();
    }

  }

  this.oldFrame = this.currentFrame;
  
});

// Making sure that the user knows they need to plug in their device 
LeapController.on('deviceConnected',function(){
  
  for( var i = 0; i < tutorials.length; i ++){
    
    if( tutorials[i].active ){
      tutorials[i].deviceConnect();
    }

  }

});

LeapController.on('deviceDisconnected',function(){
  
  for( var i = 0; i < tutorials.length; i ++){
    
    if( tutorials[i].active ){
      tutorials[i].deviceDisconnect();
    }

  }

});


LeapController.connect();


