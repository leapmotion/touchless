
var readMore = new TUTORIAL( 'readMore' , LeapController );

readMore.fingerPosition = [ 9999 , 0 ];

readMore.draw = function(){


  this.frame = LeapController.currentFrame;
  this.oFrame = LeapController.oldFrame;
  
  // Clear the last frame drawn
  this.ctx.clearRect( - this.width/2 , - this.height, this.width, this.height );
   
  for( var i = 0; i < this.buttons.length;  i++){

    this.buttons[i].checkInside(this.fingerPosition);
    this.buttons[i].draw();
    this.bsc.drawText();
    this.adv.drawText();

  }



  if( this.frame.pointables[0] && this.oFrame.pointables[0] ){
    
    this.drawFinger( this.frame.pointables[0] );

    var touching = this.frame.pointables[0].touchZone == 'touching' ;
    var oHovering = this.oFrame.pointables[0].touchZone == 'hovering'
    
    if( touching && oHovering ){
      if( this.currentButton == this.bsc ){
        console.log('bsc');
        window.location =  "bsc.html";
      }else if( this.currentButton == this.adv ){
        console.log('adv');
        window.location =  "adv.html";

      }

    } 
 
    this.fingerPosition = this.leapToScene( this.frame.pointables[0].stabilizedTipPosition );
  }

}

readMore.buttons = [];

readMore.init = function(){

  var height = this.height/4;
  var width = this.width - 20;

  var y1 = - this.height + height + 10;
  var y2 = y1 + height + 10;
  var x = 0;
  
  this.bsc = new BUTTON( this , [ x , y1 ] , width , height ); 
  this.adv = new BUTTON( this , [ x , y2 ] , width , height ); 

  this.buttons.push( this.bsc );
  this.buttons.push( this.adv );

}

readMore.init();

readMore.bsc.drawText = function(){

  var ctx = this.whichTutorial.ctx;
  if( this == this.whichTutorial.currentButton){
    ctx.fillStyle = '#eee';
  }else{
    ctx.fillStyle = '#777';
  }

  ctx.font = this.whichTutorial.width / 20 + 'px Arial';
  ctx.fillText( 'Touch and Scroll' , this.position[0], this.position[1] );

  ctx.font = this.whichTutorial.width / 40 + 'px Arial';
  ctx.fillText( 'Enables a user to touch and click with Leap Motion' , this.position[0], this.position[1] + this.whichTutorial.width/30 )


}

readMore.adv.drawText =  function(){

  var ctx = this.whichTutorial.ctx;
  if( this == this.whichTutorial.currentButton){
    ctx.fillStyle = '#eee';
  }else{
    ctx.fillStyle = '#777';
  }

  ctx.font = this.whichTutorial.width / 20 + 'px Arial';
  ctx.fillText( 'Full Touch Simulation' , this.position[0], this.position[1] );

  ctx.font = this.whichTutorial.width / 40 + 'px Arial';
  ctx.fillText( 'Maps all 10 fingers to direct touch points on the screen' , this.position[0], this.position[1] + this.whichTutorial.width/30 )


}


readMore.active = true;


