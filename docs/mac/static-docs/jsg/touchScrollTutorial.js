
var touchScroll = new TUTORIAL( 'touchScroll' , LeapController );

touchScroll.speed = [0,0] ;

touchScroll.boxes = [];

touchScroll.offset = [ 0 , 0 ];
touchScroll.size = [ 100 , 100 ];
touchScroll.speed = [ 0 , 0 ];
touchScroll.momentum = .98;

touchScroll.scrollInX = true;
touchScroll.scrollInY = true;


touchScroll.draw = function(){
  
  this.frame = LeapController.currentFrame;
  this.oFrame = LeapController.oldFrame;

  
  // Clear the last frame drawn
  this.ctx.clearRect( - this.width/2 , - this.height, this.width, this.height );
    
  this.drawGrid( '#999999' , this.offset , this.size );


  var touching = false;
  for( var i = 0; i < this.frame.pointables.length ; i++){
  
    if( this.frame.pointables[i].touchZone == 'touching' ){
      touching  = true;
    }
    this.drawFinger( this.frame.pointables[i] );

  }

  if( touching == true ){
    if( this.scrollInX ){
      this.speed[0] += this.frame.translation(this.oFrame)[0]/10;
    }
    if( this.scrollInY ){
      this.speed[1] += this.frame.translation(this.oFrame)[1]/10;
    }
  }

  this.offset[0] += this.speed[0];
  if(this.offset[0] > this.size[0]){
    this.offset[0] -= this.size[0];
  }else if( this.offset[0] < 0 ){
    this.offset[0] += this.size[0];
  }

  this.offset[1] += this.speed[1];
  if(this.offset[1] > this.size[1]){
    this.offset[1] -= this.size[1];
  }else if( this.offset[1] < 0 ){
    this.offset[1] += this.size[1];
  }


  //Momentum
  this.speed[0] *= this.momentum;
  this.speed[1] *= this.momentum;

    
  //this.drawGrid( '#ff0000' , this.offset , this.size );

}


touchScroll.drawGrid =  function( color , offset , size ){

  numberOfBarsX = this.width / size[0] + 1;
  numberOfBarsY = this.height / size[1] + 1;

  this.ctx.fillStyle = color;
  for( var i = 0; i < numberOfBarsX; i ++){
    var x = - this.width / 2 + (i * size[0] ) + offset[0];
    this.ctx.fillRect( x , 0 , 4 , -this.height); 
  }

  for( var i = 0; i < numberOfBarsY; i ++){
    var  y = 0 - (i * size[1] ) - offset[1];
    this.ctx.fillRect( -this.width/2 , y , this.width , 4); 
  }


}


touchScroll.active = true;


