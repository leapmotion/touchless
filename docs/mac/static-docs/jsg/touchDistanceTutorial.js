


var touchDistance = new TUTORIAL( 'touchDistance' , LeapController );

touchDistance.draw = function(){

  this.frame = LeapController.currentFrame;
  this.oFrame = LeapController.oldFrame;

  // Clear the last frame drawn
  this.ctx.clearRect( - this.width/2 , - this.height, this.width, this.height );

  this.ctx.fillStyle = '#ccc';
  this.ctx.fillRect( -this.width/2 ,  -this.height/2, this.width/2 , this.height/4);

  this.ctx.fillStyle = '#afa';
  this.ctx.fillRect( 0, -this.height/2 , this.width/2 , this.height/4 );


  var fontSize = 30;
  this.ctx.font = fontSize + 'px Arial';
 
  if( this.frame.pointables[0] ){

    var pointable = this.frame.pointables[0];

    var distance = -pointable.touchDistance * this.width/2;

    var size = 2

    if( distance <= 0 ){
     
      this.ctx.fillStyle = '#777';
      this.ctx.fillRect( distance , -this.height/2 , -distance, this.height/4 );
     
      this.ctx.fillStyle = '#aaa';
      
      this.ctx.fillText( 'hover' , 0  , -this.height/2 - this.height/5 - fontSize*size  );

    }else if( distance >0  ){

      this.ctx.fillStyle = '#3a3';
      this.ctx.fillRect( distance , -this.height/2  , -distance, this.height/4  );

      this.ctx.fillStyle = 'rgba( 0 ,200 , 0 , 0.4)';
      
      this.ctx.fillText( 'touch' , 0 , -this.height/2 - this.height/5 - fontSize*size  );

    }
    
    this.drawFinger( pointable , [ 0 , this.height/2 + this.height/5  ] , size );

  }

} 

touchDistance.active = true;


