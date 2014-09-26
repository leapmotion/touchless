
var numberOf = new TUTORIAL( 'numberOf' , LeapController );


numberOf.hands = [];
numberOf.fingers = [];
numberOf.frameBuffer = 20;

numberOf.draw = function(){

  this.frame = LeapController.currentFrame;
  this.oFrame = LeapController.oldFrame;

  // Clear the last frame drawn
  this.ctx.clearRect( - this.width/2 , - this.height, this.width, this.height );

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

  this.ctx.fillStyle = '#777';

  var fontSize = this.width / 5;
  this.ctx.font = fontSize + "px Arial";

  //this.ctx.fillText( hands , -this.width / 4 , -this.height/2 );
  this.ctx.fillText( fingers , -this.width / 2.7 , -this.height/2 + fontSize/4 );

  //var fontSize = 90;
  //this.ctx.font = fontSize + "px Arial";

  //this.ctx.fillText( "hands" , -this.width / 4 , -this.height/2  + fontSize/.8 );
  this.ctx.fillText( "fingers" , this.width / 8 , -this.height/2 + fontSize/4  );

}


numberOf.active = true;


