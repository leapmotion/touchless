
var touchSpeed = new TUTORIAL( 'touchSpeed' , LeapController );


// Bool to tell if we are touching or not
touchSpeed.currentlyTouching = false;

// The array we are using to keep track of the 
// first fingers position
touchSpeed.fingerPos = [0,0];

// global variable to help aide in UI creation
// to visualize touchDistance
touchSpeed.fingerSize = 20;

// The frameTipSpeeds and tipSpeedNum are how we
// save the number of frames and average over them
// in order to see how fast the user was moving
// when they were clicking, All this will feed
// into the global variable touchSpeed
touchSpeed.frameTipSpeeds = [0];
touchSpeed.tipSpeedNum = 20;
touchSpeed.touchSpeed;

// keeping an array of our buttons to store globablly
touchSpeed.buttons = [];
touchSpeed.currentButton;

// This the height of every bar.
// The offset of 8 is because of the spacing
// between bars.
touchSpeed.barHeight = (touchSpeed.height / 4)-8;

touchSpeed.touchBarOffset = 20;
touchSpeed.touchBarHeight = touchSpeed.barHeight - (touchSpeed.touchBarOffset*2);

// Colors for drawing the border of the touchBar
touchSpeed.goodColor = '#afa';
touchSpeed.badColor = '#faa';

// This is the factor we will reduce the speed by,
// to define what is a good and bad speed
touchSpeed.speedReductionFactor = 300;


// This is the function we will call every loop
// which passes frame data into our function
touchSpeed.draw = function( ) {

  // Setting up some local variables that are easier to work with 
  this.frame = this.controller.currentFrame;
  this.oFrame = this.controller.oldFrame;

  if( this.frame.gestures[0]){

    if( this.frame.gestures[0].type == 'screenTap' && this.currentlyTouching == true ){
      //TODO: Implement a 'move back' alert if neccessary 
    }

  }
  // Clear the last frame drawn
  this.ctx.clearRect( - this.width/2 , - this.height, this.width, this.height );

  // Makes sure that we have the first finger from both this frame
  // and the previous frame
  if( this.frame.pointables[0] && this.oFrame.pointables[0] ){

    var pointable = this.frame.pointables[0];
    var oPointable = this.oFrame.pointables[0];

    // Getting the pointables position
    var pos = this.leapToScene(pointable.stabilizedTipPosition);

    // Sets out finger position and fingerSize
    this.fingerPos = pos;
    this.fingerSize = pointable.touchDistance;

    /* 
    
      Sets up some boolean tests, thats are used
      to trigger the 'click' event.

    */

    // The first test is to see if the current touchZone
    // and the old touchZone are different
    var difZones = pointable.touchZone != oPointable.touchZone;

    // If they are different, and the current touchZone is
    // touching, that means that we have switched from 
    // hovering to touching
    var touching = pointable.touchZone == 'touching';

    // If we have clicked a button
    if( difZones && touching){
      
      this.currentlyTouching = true;
      
      if( this.currentButton ){

        // Gets the average from the array of tipSpeeds that
        // we have been saving
        this.touchSpeed = this.getAverageFromArray( this.frameTipSpeeds );

        // Sets the currentButtons clickColor based
        // on the touchSpeed
        this.currentButton.clickBoxColor = this.getColorFromSpeed();

      }
      
    }else if( difZones && !touching ){

      this.currentlyTouching  = false;

    }

  }
  
  // Adds our current frameTip to the tipSpeeds array
  // If there is no tip, add a zero to the array
  if( this.frame.pointables[0] ){
    
    this.frameTipSpeeds[0] = this.frame.pointables[0].tipVelocity[2];
    
  }else{
  
    this.frameTipSpeeds[0] = 0
  
  }

  // Pushes all of the velocities up 1 in the array
  for( var i = this.tipSpeedNum; i >= 1; i --){
    
    if(this.frameTipSpeeds[i-1]){
      this.frameTipSpeeds[i] = this.frameTipSpeeds[i-1];
    }else{
      this.frameTipSpeeds[i] = 0;
    }

  }
  

  for( var i = 0; i < this.buttons.length;  i++){

    this.buttons[i].checkInside(this.fingerPos);
    this.buttons[i].draw();

  }

  // Draws the touchBar and the finger
  this.drawTouchBar();
  if( this.frame.pointables[0]){
    this.drawFinger(this.frame.pointables[0]);
  }

};



// Function to draw the touchBar
touchSpeed.drawTouchBar = function(){
  this.drawTouchBarInfoText();
  
  this.ctx.fillStyle = this.getFillFromRGBA(this.getColorFromSpeed());

  var left = - this.width / 2;
  var right = this.getSpeedValue() * this.width;
  this.ctx.fillRect( left+4 , this.touchBarTop + this.touchBarHeight/2 + 8 , right , -this.touchBarHeight+4 );

  this.drawTouchBarBorder();
  this.drawTouchBarText();

}

touchSpeed.drawTouchBarInfoText = function(){
  var fontSize = this.touchBarOffset / 1.2;
  
  this.ctx.font = fontSize + 'px Arial';
  this.ctx.fillStyle = '#4B4B4B'
  this.ctx.textAlign = 'center';
  this.ctx.fillText( 'Touch Speed - Lower is Better', 6, this.touchBarTop - this.touchBarHeight/2   );
  
}

// Drawing the touch bar border
touchSpeed.drawTouchBarBorder = function(){

  var z =  - this.width /2;
  var bottom = this.touchBarTop + this.touchBarHeight/2 + 10;  
  var top = bottom - this.touchBarHeight;
  var left = z+1;
  var middle = 0;
  var right = this.width/2 - 2;

  this.ctx.strokeStyle = this.goodColor;
  this.ctx.lineWidth = 4;
  //Draw good parts
  //this.ctx.strokeRect( z+1  , bottom , this.width/2 -4 , -this.touchBarHeight);
  this.ctx.beginPath();
  //this.ctx.moveTo( (z+1 +(this.width/2) - 4) / 2, bottom - this.touchBarHeight);
  this.ctx.moveTo( middle, top);
  this.ctx.lineTo( left, top);
  this.ctx.lineTo( left, bottom );
  this.ctx.lineTo( middle, bottom );
  this.ctx.stroke();
  this.ctx.beginPath();
  this.ctx.strokeStyle = this.badColor;
  this.ctx.moveTo( middle, bottom );
  this.ctx.lineTo( right, bottom );
  this.ctx.lineTo( right, top );
  this.ctx.lineTo( middle, top );
  this.ctx.stroke();
  //this.ctx.strokeRect( 1  , bottom , this.width/2 - 2 , -this.touchBarHeight);
}


// drawing the touch bar text
touchSpeed.drawTouchBarText = function(){

  var fontSize = this.touchBarHeight / 3;

  this.ctx.font = fontSize + 'px Arial';

  var speedValue = this.getSpeedValue();
  
  if( speedValue  <= .5 ){
    this.ctx.fillStyle = '#393';
	this.ctx.textAlign = 'center';
    this.ctx.fillText( 'Good' ,  -this.width/ 4 , -this.touchBarHeight/2 + fontSize/2 - 4 );
  }else if( speedValue  > .5 ){
       this.ctx.fillStyle =  '#833';
    this.ctx.fillText( 'Try Smaller Movements' , this.width/ 4, -this.touchBarHeight /2 + fontSize/2 - 4 );
  }
}


/*

  Helper Functions

*/

touchSpeed.getColorFromSpeed =  function(){

  var nTouchSpeed = this.getSpeedValue();

  if( nTouchSpeed <= .5 ){
    return { r:80 , g: 200 , b:80 , a:1 }
  }else{
    return { r:200 , g: 80 , b:80 , a:1 }
  }
}

touchSpeed.getSpeedValue = function() {
  return Math.abs(this.touchSpeed)/this.speedReductionFactor;
}

touchSpeed.init = function(){

/*

  Function for drawing buttons ( which could be a million times prettier )

*/

this.buttons = [];

for( var i = 0; i <2 ; i ++ ){

  var width = (this.width/2) - 5;
  var height = this.barHeight;

  var x =( i * (width + 10)) + width/2    - (this.width/2 );
  var y = - this.height + height/2;

  var button = new BUTTON( this , [x,y] , width, height );
  this.buttons.push( button );

  for( var j = 0; j < 2; j ++){

    width1 = (width / 2) - 5 ;
    x1 = x-5 + ( j * (width1 + 10) ) - width1/2 ;
    y1 = y + height + 10;

    var button = new BUTTON( this , [x1,y1] , width1, height );
    this.buttons.push( button );

    for( var k = 0; k < 2; k ++){

      width2 = (width1 / 2) - 5 ;
      x2 = x1 - 5 + ( k * (width2 + 10) ) - width2/2 ;
      y2 = y1 + height + 10;
      var button = new BUTTON( this , [x2,y2] , width2, height );
      this.buttons.push( button );
      this.touchBarTop = y2 + height + this.touchBarOffset;
    
    }

  }

}


}



touchSpeed.init();
touchSpeed.active = true;

