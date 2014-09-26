

  /*
    
      Button Object

  */

  // Each button needs to be clickable, and hoverable
  function BUTTON( whichTutorial, position, width, height ){

    this.whichTutorial = whichTutorial;

    // Defintion of size of button
    this.position = position;
    this.width = width;
    this.height = height;

    // Attributes for collision detection
    this.left = position[0] - width/2;
    this.right = position[0] + width/2;
    this.top = position[1] + height/2;
    this.bottom = position[1] - height/2;

    // Set the color of the button to the unhovered colo 
    this.color = {
      r:200,
      g:200,
      b:200,
      a:.9
    }

    // Sets up the color of the box that will always be
    // drawn on top
    this.clickBoxColor = {
      r:0,
      g:0,
      b:0,
      a:0
    }

  }

  BUTTON.prototype = {


    // Checks to see if a certain position is 
    // inside the button
    checkInside: function( position ){

      var inLeft = position[0] >= this.left;
      var inRight = position[0] <= this.right;
      var inTop =  -position[1] <= this.top;
      var inBottom = -position[1] >= this.bottom; 


      // ' Mouse Over ' function
      if( inLeft && inRight && inTop && inBottom){

        // If the color is currently our unhovered color
        // Switch it to the hovered color
        if(this.color.b == 200){
          this.color.r = 150;
          this.color.g = 150;
          this.color.b = 150;
        }

        // Sets the current button to this button
        // So that we can assign the clickBoxColor
        // if we click on this button
        this.whichTutorial.currentButton = this;

      // ' Mouse Out ' function
      }else{

        // Makes sure that if there is no button 
        // being hovered over, there is no current
        // button
        if( this.whichTutorial.currentButton == this ){
          this.whichTutorial.currentButton = undefined;
        }

        // If the color is the hovered color,
        // Switch it to the unhovered color
        if( this.color.b == 150 ){
          this.color.r = 200;
          this.color.g = 200;
          this.color.b = 200;
        }
      
      }

    },

    draw: function(){

      // Draws the main button
      this.whichTutorial.ctx.fillStyle = this.whichTutorial.getFillFromRGBA(this.color);
      this.whichTutorial.ctx.fillRect( this.left, this.bottom , this.width , this.height);

      // Draws the clicked Box
      this.whichTutorial.ctx.fillStyle = this.whichTutorial.getFillFromRGBA( this.clickBoxColor );
      this.whichTutorial.ctx.fillRect( this.left, this.bottom , this.width , this.height);

      // Fades out the clickBoxColor
      if( this.clickBoxColor.a >= 0){
        this.clickBoxColor.a -= .01;
      }

    }

  }


