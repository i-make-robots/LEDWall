class DemoTransmitter extends Thread {

  int animationStep = 0;
  int stepSize = 10;

  color[] MakeDemoFrame() {
    int image_size = ledCount; 

    color[] imageData = new color[image_size];

    for (int i = 0; i < imageData.length; i++) {
      if (animationStep == i % stepSize) {
        if (i >= int(imageData.length*2/3.0)) {
          imageData[i] = color(0, 0, 128);
        } 
        else if (i >= int(imageData.length/3.0)) {
          imageData[i] = color(0, 128, 0);
        } 
        else {
          imageData[i] = color(128, 0, 0);
        }
      }
      else {
        imageData[i] = color(0, 0, 0);
      }
    }

    animationStep = (animationStep + 1)%stepSize;

    return imageData;
  }

  DemoTransmitter() {
  }

  void run() {
    while (demoMode) {
      try {
        if (newImageQueue.size() < 1) {
          color imageData[] = MakeDemoFrame();
          newImageQueue.put(imageData);
        }
        Thread.sleep(250);
      } 
      catch( InterruptedException e ) {
        println("Interrupted Exception caught");
      }
    }
  }
}

