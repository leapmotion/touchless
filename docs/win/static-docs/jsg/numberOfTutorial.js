
function numberOfTutorial ( tutorialName , controller ){

  
  this.containerId = tutorialName + 'Tutorial';
  this.containerSelector = '#' + this.containerId;
  this.container = $( this.containerSelector );

  this.height =  window.innerHeight * .75;
  this.width = this.height * 1.5;

  this.container.css('height' , this.height +"px");

  this.plugInDeviceId = tutorialName + 'PlugInDevice';
  
  this.plugInDeviceJ =
    $('<img/>',{ 'id': this.plugInDeviceId, 'src' : 'plugInDevice.png'})
      .width('60%');

  this.plugInDevice = this.plugInDeviceJ[0];

  this.plugInDevice.style.display = 'none';

  this.interactiveArea = $('#interactiveArea')[0];

  var self = this;
  var t = window.setTimeout(function(){
    if( !self.pluggedIn ){
      self.plugInDevice.style.display = 'block';
      self.interactiveArea.style.display = 'none';
    }
  },1000)

  this.container.append( this.plugInDevice );


  /*
   *    Setting up DOM element controllers
   */


  this.finger1 = $('#finger1');
  this.finger2 = $('#finger2');
  this.finger3 = $('#finger3');

  this.numberOfFingers = $('#numberOfFingers');
  this.numberOfFingers.css('font-size' , this.width/20 + "px");
  this.numberOfFingersHeader = $('#numberOfFingersHeader');

  this.touch = $('#touch');
  this.scroll = $('#scroll');

  this.clickScroll = $('#clickScroll');
  this.zoomRotate = $('#zoomRotate');
  this.drawDiv = $('#draw');

  this.controller = controller;
  tutorials.push( this );

}

numberOfTutorial.prototype = {


  deviceConnect:function(){
    this.pluggedIn = true;
    this.interactiveArea.style.display = 'block';
    this.plugInDevice.style.display = 'none';
  },

  deviceDisconnect:function(){
    this.pluggedIn = true;
    this.interactiveArea.style.display = 'none';
    this.plugInDevice.style.display = 'block';
  },

  getAverageFromArray: function( a ){
    var total = 0 ;
    for( var i = 0; i < a.length; i++){
      total += a[i];
    }
    var ave = total / a.length;
    return ave;
  },


}

numberOf = new numberOfTutorial( 'numberOf' , LeapController );

// Arrays to keep the number of hands
numberOf.hands = [];
numberOf.fingers = [];
numberOf.frameBuffer = 20;




numberOf.draw = function(){

  this.frame = LeapController.currentFrame;
  this.oFrame = LeapController.oldFrame;


  this.hands[0] = this.frame.hands.length;
  this.fingers[0] = this.frame.pointables.length;

  for( var i = this.frameBuffer; i > 0; i-- ){

    if( this.hands[i-1] ){
      this.hands[i] = this.hands[i-1];
      this.fingers[i] = this.fingers[i-1];
    }else{
      this.hands[i] = 0;
      this.fingers[i] = 0;
    }

  }

  var hands = Math.round( this.getAverageFromArray( this.hands ) );
  var fingers = Math.round( this.getAverageFromArray( this.fingers ) );

  this.numberOfFingers.html( function(){
    return fingers + " Fingers"
  })

  if( fingers > 0 ){
    this.numberOfFingers.css( 'color' , '#afa' );
  }else{
    this.numberOfFingers.css( 'color' , '#ccc' ); 
  }

  // Clears backgrounds
  this.finger1.css('background','none');
  this.finger2.css('background','none');
  this.finger3.css('background','none');
  this.touch.css('background','none');
  this.scroll.css('background','none');
  this.clickScroll.css('background','none');
  this.zoomRotate.css('background','none');
  this.drawDiv.css('background','none');

  if( fingers == 1){
 
    
    this.finger1.css('background','#afa');
    this.touch.css('background','#afa');
    this.clickScroll.css('background','#afa');
  
  }else if( fingers == 2){
  
    this.finger2.css('background','#afa');
    this.scroll.css('background','#afa');
    this.zoomRotate.css('background','#afa');

  }else if( fingers > 2){
  
    this.finger3.css('background','#afa');
    this.scroll.css('background','#afa');
    this.drawDiv.css('background','#afa');

  }



  var fontSize = this.width / 5;
  //this.ctx.font = fontSize + "px Arial";

  //this.ctx.fillText( hands , -this.width / 4 , -this.height/2 );
  //this.ctx.fillText( fingers , -this.width / 2.7 , -this.height/1.5 );
  //this.ctx.fillText( "fingers" , this.width / 8 , -this.height/1.5 );


  //var fontSize = 90;
  //this.ctx.font = fontSize + "px Arial";
  //this.ctx.fillText( "hands" , -this.width / 4 , -this.height/2  + fontSize/.8 );

}
numberOf.active = true;

