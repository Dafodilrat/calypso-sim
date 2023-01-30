#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/Float64.h>

#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include <map>
using namespace ros;

// /auv/BLDC_1_motor_controller/command
// /auv/BLDC_2_motor_controller/command
// /auv/BLDC_3_motor_controller/command
// /auv/BLDC_4_motor_controller/command
// /auv/BLDC_5_motor_controller/command
// /auv/BLDC_6_motor_controller/command
// /auv/BLDC_7_motor_controller/command
// /auv/BLDC_8_motor_controller/command

// 1:right upper 
// 2:left upper
// 3:left lower
// 4:right lower
// 5:front left
// 6:back lower
// 7:back left
// 8:front right

float BLDC_1=0.0;
float BLDC_2=0.0;
float BLDC_3=0.0;
float BLDC_4=0.0;
float BLDC_5=0.0;
float BLDC_6=0.0;
float BLDC_7=0.0;
float BLDC_8=0.0;
float MIN_THRUST=0.0;
float MAX_THRUST=90.0;
float LIFT=0;
float gain=5;


Publisher PBLDC_1;
Publisher PBLDC_2;
Publisher PBLDC_3;
Publisher PBLDC_4;
Publisher PBLDC_5;
Publisher PBLDC_6;
Publisher PBLDC_7;
Publisher PBLDC_8;

char key(' ');

float check(float x)
{
    
    if(x<=0)
    {
        return 0;
    }
    
    if (x>=MAX_THRUST)
    {
        return MAX_THRUST;
    }
    return x;
}

class movements
{
    public:
    void PublishV()
    {
        std_msgs::Float64 temp;
        
        temp.data=check(BLDC_1);
        PBLDC_1.publish(temp);

        temp.data=check(BLDC_2);
        PBLDC_2.publish(temp); 

        temp.data=check(BLDC_3);
        PBLDC_3.publish(temp);

        temp.data=check(BLDC_4);
        PBLDC_4.publish(temp);

        temp.data=check(BLDC_5);
        PBLDC_5.publish(temp);

        temp.data=check(BLDC_6);
        PBLDC_6.publish(temp);

        temp.data=check(BLDC_7);
        PBLDC_7.publish(temp);

        temp.data=check(BLDC_8);
        PBLDC_8.publish(temp);


    }

    void reset_z()
    {
        BLDC_1=LIFT;
        BLDC_2=LIFT;
        BLDC_3=LIFT;
        BLDC_4=LIFT;
    }

    void ZThrust(float x)
    {
        float y=x*gain;
        LIFT+=y;
        LIFT+=y;
        LIFT+=y;
        LIFT+=y;
        BLDC_1=LIFT;
        BLDC_2=LIFT;
        BLDC_3=LIFT;
        BLDC_4=LIFT;

        PublishV();
    }

    void Pitch(float x)
    {
        float y=x*gain;
        if(x>0)
        {
            reset_z();
            BLDC_1+=y;
            BLDC_2+=y;

        }
        else
        {
            reset_z();
            BLDC_3+=y;
            BLDC_4+=y;
        }

        PublishV();
    }

    void Roll(float x)
    {
        float y=x*gain;
        if(x>0)
        {
            // reset_z();
            BLDC_1+=y;
            BLDC_4+=y;

            // if(BLDC_6 >0 && BLDC_7 >0)
            // {
            //     LIFT=BLDC_1;
            // } 
        }
        else
        {
            // reset_z();
            BLDC_2+=y;
            BLDC_3+=y;
            
            // if(BLDC_5 >0 && BLDC_8 >0)
            // {
            //     LIFT=BLDC_2;
            // } 
        }
        PublishV();
    }

    void Front_Back(float x)
    {
        float y=x*gain;
        if(x>0)
        {
            BLDC_5=0;
            BLDC_8=0;
            BLDC_7+=y;
            BLDC_6+=y;
        }
        else
        {
            BLDC_7=0;
            BLDC_6=0;
            BLDC_8+=y;
            BLDC_5+=y;
        }
        PublishV();
    }
    
    void left_Right(float x) 
    {
        float y=x*gain;
        if(x>0)
        {
            BLDC_5+=y;
            BLDC_6+=y;
            BLDC_7=0;
            BLDC_8=0;
        }
        else
        {
            BLDC_5=0;
            BLDC_6=0;
            BLDC_7+=y;
            BLDC_8+=y;
        }
    }   

};

const char* msg = R"(

Reading from the keyboard and Publishing to Twist!
---------------------------
Moving around:
   w :move up
   s :move down
   i :pitch up
   k :pitch down
   j :yaw left
   k :yaw right
   
For Holonomic mode (strafing), hold down the shift key:
---------------------------
   U    I    O
   J    K    L
   M    <    >

t : up (+z)
b : down (-z)

anything else : stop

q/z : increase/decrease max speeds by 10%
w/x : increase/decrease only linear speed by 10%
e/c : increase/decrease only angular speed by 10%

CTRL-C to quit

)";


int getch(void)
{
  int ch;
  struct termios oldt;
  struct termios newt;

  // Store old settings, and copy to new settings
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;

  // Make required changes and apply the settings
  newt.c_lflag &= ~(ICANON | ECHO);
  newt.c_iflag |= IGNBRK;
  newt.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF);
  newt.c_lflag &= ~(ICANON | ECHO | ECHOK | ECHOE | ECHONL | ISIG | IEXTEN);
  newt.c_cc[VMIN] = 1;
  newt.c_cc[VTIME] = 0;
  tcsetattr(fileno(stdin), TCSANOW, &newt);

  // Get the current character
  ch = getchar();

  // Reapply old settings
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  return ch;
}


int main(int argc, char **argv)
{
    init(argc, argv, "teleop_node");

    NodeHandle n;
    Rate loop_rate(10);

    printf("%s", msg);

    PBLDC_1 = n.advertise<std_msgs::Float64>("/prop_1_to_thruster_1_joint_controller/command", 1000);
    PBLDC_2 = n.advertise<std_msgs::Float64>("/prop_2_to_thruster_2_joint_controller/command", 1000);
    PBLDC_3 = n.advertise<std_msgs::Float64>("/prop_3_to_thruster_3_joint_controller/command", 1000);
    PBLDC_4 = n.advertise<std_msgs::Float64>("/prop_4_to_thruster_4_joint_controller/command", 1000);
    PBLDC_5 = n.advertise<std_msgs::Float64>("/prop_5_to_thruster_5_joint_controller/command", 1000);
    PBLDC_6 = n.advertise<std_msgs::Float64>("/prop_6_to_thruster_6_joint_controller/command", 1000);
    PBLDC_7 = n.advertise<std_msgs::Float64>("/prop_7_to_thruster_7_joint_controller/command", 1000);
    PBLDC_8 = n.advertise<std_msgs::Float64>("/prop_8_to_thruster_8_joint_controller/command", 1000);
    movements auv;

    while (true)
    {
        int c = getch();

        if (c == 'y')
        {
            auv.ZThrust(1);
        }

        else if (c == 'h')
        {
            auv.ZThrust(-1);
        }
        else if (c == 'i')
        {
            auv.Pitch(1);   
        }
        else if (c == 'k')
        {
            auv.Pitch(-1);
        }
        else if (c == 'l')
        {
            auv.Roll(1);
        }
        else if (c == 'j')
        { 
            auv.Roll(-1);
        }
        else if (c == '\x03')
        {
            printf("\n\n........ ABORTING!!!! .......... ABORTING!!!! ........ \n\n");
            break;
        }
        else if (c == 'a')
        { 
            auv.left_Right(1);
        }
        else if (c == 'd')
        { 
            auv.left_Right(-1);
        }
        else if (c == 'w')
        { 
            auv.Front_Back(1);
        }
        else if (c == 's')
        { 
            auv.Front_Back(-1);
        }

        spinOnce();
        loop_rate.sleep();
    }
}