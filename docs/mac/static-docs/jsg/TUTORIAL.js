

/*
 *  Creating a TUTORIAL object requires a div to be placed inside
 *  and will create the canvas itself. The height and width of the 
 *  example will be defined by the container size
 *
 */
TUTORIAL = function( tutorialName , controller ){

  this.containerId = tutorialName + 'Tutorial';
  this.containerSelector = '#' + this.containerId;
  this.container = $( this.containerSelector );

  this.height =  window.innerHeight * .8;
  this.width = this.height * 1.5;


  this.container.height = this.width;
  this.container.width = this.height;

  this.container.css('width', this.width +"px");
  this.container.css('height', this.height +"px");

  console.log('width')

  this.canvasId = tutorialName + 'Canvas';
  this.plugInDeviceId = tutorialName + 'PlugInDevice';
  
  // The Jquery Object for this canvas
  this.canvasJ = 
    $('<canvas/>',{'id':this.canvasId})
    .width(this.width)
    .height(this.height)
  
  // the dom object for this canvas
  this.canvas = this.canvasJ[0];
  this.canvas.width = this.width;
  this.canvas.height = this.height;

  this.canvas.style.display = 'none';

  this.plugInDeviceJ =
    $('<img/>',{ 'id': this.plugInDeviceId, 'src' : 'plugInDevice.png'})
      .width('80%')

  this.plugInDevice = this.plugInDeviceJ[0];

  this.plugInDevice.style.display = 'none';

  var self = this;
  var t = window.setTimeout(function(){
    if( !self.pluggedIn ){
      self.plugInDevice.style.display = 'block';
    }
  },1000)


  // sets of up the context
  this.ctx = this.canvas.getContext( '2d' );
  
  // Setting the zero of the canvas, so that is corresponds
  // with the leap coordinate system
  this.ctx.translate( this.canvas.width/2 , this.canvas.height );
 
  this.container.append( this.canvas );
  this.container.append( this.plugInDevice );

  this.fingerSize = 0;


  // Setting up text for context
  this.ctx.textAlign = 'center' ;
  this.ctx.font = '15px Arial';


  this.controller = controller ;

  // Finally pushes this tutorial to the tutorials array
  tutorials.push( this );

}


TUTORIAL.prototype = {



  deviceConnect:function(){

    this.pluggedIn = true;
    this.canvas.style.display = 'block';
    this.plugInDevice.style.display = 'none';

  },

  deviceDisconnect:function(){

    this.pluggedIn = false;
    this.canvas.style.display = 'none';
    this.plugInDevice.style.display = 'block';

  },

  leapToScene: function(pos){
   
    iBox = this.frame.interactionBox;
    x = ( pos[0]/iBox.width )* this.width;
    y = ( ( pos[1] - (iBox.center[1] - iBox.height/2) )/(iBox.height) )  * this.height;
    return [x,y]   

  },


  /*
   *  draws a finger with the proper coloring and size 
   *  based on its touch distance. If a position is passed in
   *  it will draw it at the position defined, otherwise, it will
   *  draw it at the stablized tipPosition
   *
   */
  drawFinger: function(pointable, pos , sizeRatio ){

    // The size of the circle we are drawing at the fingertip 
    // position
    var fingerCircleSize;
    var fingerSize = pointable.touchDistance;
    var position;

    var size = 1;

    if( sizeRatio ){
      var size = sizeRatio;
    } 
    

    if( pos ){
      position = pos; 
    }else{
      position = this.leapToScene( pointable.stabilizedTipPosition );
    }

    if(position[0] <= -this.width/2 ){

      position[0] = -this.width/2;

    }

    if(position[0] >= this.width/2 ){

      position[0] = this.width/2;

    }

    if(position[1] >= this.height){

      position[1] = this.height;

    }

    if(position[1] <= 0){

      position[1] = 0;

    }


    // If the finger is 'clicking'
    // draw a circle of a specific size and some blur
    if( fingerSize <= 0 ){

      fingerCircleSize = 10 * size;
      
      // The circle should be filled with slightly opaque green
      this.ctx.fillStyle = 'rgba( 0 ,200 , 0 , 0.4)';
      this.ctx.shadowColor = '#0a0';
      this.ctx.strokeStyle = '#ffffff';
      this.ctx.lineWidth = 4 * size;
      this.ctx.shadowBlur = 50;

    // Otherwise draw the finger based on how far away we
    // are from the Touch Plane
    }else{
   

      // The circle should be filled with slightly opaque white
      this.ctx.fillStyle = 'rgba( 255 ,255 ,255 , 0.3)';
      this.ctx.strokeStyle = '#777777';
      this.ctx.lineWidth = 8 * size;
      fingerCircleSize = 4 + fingerSize * 50 * size;
      
      if( fingerCircleSize >= 20*size ){
        fingerCircleSize = 20*size;
      }

    }

    this.ctx.beginPath();
    this.ctx.arc( position[0], - position[1] , fingerCircleSize , 0 , 2*Math.PI );
    this.ctx.fill();
    this.ctx.stroke();

    //Makes sure that our shadowBlur is set back to 0
    this.ctx.shadowBlur = 0;


  },


  getAverageFromArray: function( a ){
    var total = 0 ;
    for( var i = 0; i < a.length; i++){
      total += a[i];
    }
    var ave = total / a.length;
    return ave;
  },

  getFillFromRGBA: function(c){
    return "rgba("+ c.r + "," + c.g + "," + c.b + "," + c.a + ")";
  }


}

