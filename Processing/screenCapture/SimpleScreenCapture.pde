// https://github.com/onformative/ScreenCapturer/issues/2
import java.awt.Robot;
import java.awt.Rectangle;
import java.awt.AWTException;

class SimpleScreenCapture {
  Robot robot;
  PImage screenshot;
  
  SimpleScreenCapture() {
    try {
      robot = new Robot();
    }
    catch (AWTException e) {
      println(e);
    }
  }

  PImage get() {
    screenshot = new PImage(robot.createScreenCapture(new Rectangle(50, 50, width, height)));
    return screenshot;
  }

}
