#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

// arduino pins and I2C lcd pins
// GND (GND); 5V (Vcc); A4 (SDA); A5(SCL);  

// 12  PB4  button up
// A1  button down
// A2  button left
// A3  button right

// rows
#define r2 0
#define r3 1
#define r4 2
#define r5 3
#define r6 4
#define r7 5

// columns
#define c2 6
#define c3 7
#define c4 8
#define c5 9
#define c6 10
#define c7 11

struct matrix_cell
{
  int row;
  int column;
};

matrix_cell led_array[6][6]; // six rows, six coumns
matrix_cell snake[36]; // snake array
matrix_cell fruit; // position of fruit

int snake_len = 2; // length of snake

int s_direction = 4; // direction of snake 0=up; 1=down; 2=left; 3=right;
                      // 4=in start_position; 5 = game over;
int i; // for loops
int k; // for loops
bool snake_ate = false;     // snake ate fruit
bool fruit_snake = false;   // fruit position is the same as snake position
bool snake_crossed = false; // snake crossed herself

void snake_draw(void);      // draw snake and fruit
void snake_up(void);        // snake goes up
void snake_down(void);      // snake goes down
void snake_left(void);      // snake goes left
void snake_right(void);     // snake goes right
void fruit_gener(void);     // generate new coordinates of fruit
void snake_ate_fruit(void); // check if snake ate fruit
void snake_cross(void);     // check if snake crossed herself

void setup()
{
  pinMode(A0, INPUT);
  PORTC &= 0b11111110; // turn off pullup resitsor
  randomSeed(analogRead(0));
  
  cli(); // interrupts forbidden
  
  pinMode(r2, OUTPUT);
  pinMode(r3, OUTPUT);
  pinMode(r4, OUTPUT);
  pinMode(r5, OUTPUT);
  pinMode(r6, OUTPUT);
  pinMode(r7, OUTPUT);

  pinMode(c2, OUTPUT);
  pinMode(c3, OUTPUT);
  pinMode(c4, OUTPUT);
  pinMode(c5, OUTPUT);
  pinMode(c6, OUTPUT);
  pinMode(c7, OUTPUT);

  for(i=0; i<6; i++)
  {
    for(k=0; k<6; k++)
    {
      led_array[i][k].row = i;
      led_array[i][k].column = 6+k; 
      digitalWrite(led_array[i][k].row, LOW); // turn off all leds
      digitalWrite(led_array[i][k].column, HIGH); // turn off all leds
    }
  }

  pinMode(12, INPUT); // 12, PB4,  button up
  pinMode(A1, INPUT); // A1, PC1,  button down
  pinMode(A2, INPUT); // A2, PC2,  button left
  pinMode(A3, INPUT); // A3, PC3,  button right

  // pull up resistors for A3, A2, A1 inputs
  PORTC |= (1 << PORTC3) | (1 << PORTC2) | (1 << PORTC1);

  // inputs as Pin Change interrupts
  PCMSK1 |= (1 << PCINT11) | (1 << PCINT10) | (1 << PCINT9);

  // Pin Change interrupts enabled PCINT11, PCINT10, PCINT9
  PCICR |= (1 << PCIE1);

  PORTB |= (1 << PORTB4);
  PCMSK0 |= (1 << PCINT4);
  PCICR |= (1 << PCIE0);

  snake[0].row = 0;
  snake[0].column = 1;
  snake[1].row = 0;
  snake[1].column = 0;  
  
  fruit.row = 5;
  fruit.column = 5;
  fruit_gener();

  snake_draw();  
  
  sei(); // interrupts are enabled

  lcd.begin();
  lcd.backlight(); 
}


void loop()
{
  switch(s_direction) // direction of snake 0=up; 1=down; 2=left; 3=right
  {
    case 0:
    snake_up();
    break;

    case 1:
    snake_down();
    break;

    case 2:
    snake_left();
    break;

    case 3:
    snake_right();
    break;

    case 4: // whait for starting
    lcd.setCursor(0, 0); // put lcd cursor, 0 column, 1 row
    lcd.print("Snake, press any");
    lcd.setCursor(0, 1); // put lcd cursor, 0 column, 2 row
    lcd.print("button to begin!");
    delay(250);
    snake_draw();
    lcd.clear();
    snake_draw();
    break;

    case 5:
    lcd.setCursor(0, 0); // put lcd cursor, 0 column, 1 row
    lcd.print("   Game over!   ");
    delay(500);
    lcd.setCursor(0, 0);
    lcd.print("                ");
    delay(500);
    break;

    default:
    break;
  }
  

  if(s_direction != 4 and s_direction!=5)
  {
    lcd.setCursor(0, 0); // put lcd cursor, 0 column, 1 row
    lcd.print("snake is running");
    lcd.setCursor(0, 1);
    lcd.print("Your score is ");
    lcd.print(snake_len-2);
    lcd.print(' ');
    
    snake_cross();
    
    if(snake_crossed or snake_len == 36)
    {
      s_direction = 5; // game over
    }
    
    snake_ate_fruit();
    if(snake_ate and s_direction != 5)
    {
       fruit_gener();
       snake_ate = false;
    }
    
    snake_draw();
      
  }

}

void snake_draw(void)
{
  for(k = 0; k < int(100/snake_len); k++)
  {    
    for(i=0; i<snake_len; i++)
    {
      digitalWrite(led_array[0][0].row, LOW);     // problems with diode [0;0]
      digitalWrite(led_array[0][0].column, HIGH); // problems with diode [0;0]
      digitalWrite(led_array[snake[i].row][snake[i].column].row, HIGH);
      digitalWrite(led_array[snake[i].row][snake[i].column].column, LOW);
      delay(1);
      digitalWrite(led_array[0][0].row, LOW);     // problems with diode [0;0]
      digitalWrite(led_array[0][0].column, HIGH); // problems with diode [0;0]
      digitalWrite(led_array[snake[i].row][snake[i].column].row, LOW);
      digitalWrite(led_array[snake[i].row][snake[i].column].column, HIGH);
      delay(1);
    }
    digitalWrite(led_array[0][0].row, LOW);     // problems with diode [0;0]
    digitalWrite(led_array[0][0].column, HIGH); // problems with diode [0;0]
    digitalWrite(led_array[fruit.row][fruit.column].row, HIGH);
    digitalWrite(led_array[fruit.row][fruit.column].column, LOW);
    delay(1);
    digitalWrite(led_array[0][0].row, LOW);     // problems with diode [0;0]
    digitalWrite(led_array[0][0].column, HIGH); // problems with diode [0;0]
    digitalWrite(led_array[fruit.row][fruit.column].row, LOW);
    digitalWrite(led_array[fruit.row][fruit.column].column, HIGH);
    delay(1);
  }
}


void snake_up(void)
{
  for(i = snake_len-1; i>0; i--)
  {
    snake[i] = snake[i-1];
  }
  snake[0].row -= 1;

  if(snake[0].row < 0)
  {
    s_direction = 5;
  }
}


void snake_down(void)
{
  for(i = snake_len-1; i>0; i--)
  {
    snake[i] = snake[i-1];
  }
  snake[0].row += 1;

  if(snake[0].row > 5)
  {
    s_direction = 5;
  }
}


void snake_left(void)
{
  for(i = snake_len-1; i>0; i--)
  {
    snake[i] = snake[i-1];
  }
  snake[0].column -= 1;

  if(snake[0].column < 0)
  {
    s_direction = 5;
  }
}


void snake_right(void)
{
  for(i = snake_len-1; i>0; i--)
  {
    snake[i] = snake[i-1];
  }
  snake[0].column += 1;
  
  if(snake[0].column > 5)
  {
    s_direction = 5;
  }
}


void fruit_gener(void)
{
  fruit_snake = false;
  fruit.row = int(random(6));
  fruit.column = int(random(6));
  
  for(i=0; i<snake_len; i++)
  {
    if(snake[i].row == fruit.row and snake[i].column == fruit.column)
    {
      fruit_snake = true;
      break;
    }
  }

  while(fruit_snake)
  {
    fruit_gener();
  }
}


void snake_ate_fruit(void)
{
  if(snake[0].row == fruit.row and snake[0].column == fruit.column)
  {
    snake_len++;
    snake_ate = true;
  } 
}


void snake_cross(void)
{
  for(k=0; k<snake_len; k++)
  {
    for(i=1+k; i<snake_len; i++)
    {
      if(snake[k].row == snake[i].row and snake[k].column == snake[i].column)
      {
        snake_crossed = true;
        break;
      }
    }
  }
}


ISR(PCINT1_vect)
{ 
  delay(50);
 
  if(!(PINC & 0x02) and s_direction!=4 and s_direction!=5)// A1, PC1,  button down
  {
    //snake_head.row += 1;
    if(snake[0].row != snake[1].row - 1) // direction of snake 0=up; 1=down; 2=left; 3=right
    {
      s_direction = 1; // direction of snake 1=down; 
    }
  }
  else if(!(PINC & 0x04) and s_direction!=4 and s_direction!=5)// A2, PC2,  button left
  {
    //snake_head.column -= 1;
    if(snake[0].column != snake[1].column + 1) // direction of snake 0=up; 1=down; 2=left; 3=right
    {
      s_direction = 2; // direction of snake 2=left; 
    }
    
  }
  else if(!(PINC & 0x08) and s_direction!=4 and s_direction!=5)// A3, PC3,  button right
  {
    //snake_head.column += 1;
    if(snake[0].column != snake[1].column - 1) // direction of snake 0=up; 1=down; 2=left; 3=right
    {
      s_direction = 3; // direction of snake 2=left; 
    }
  }
  else if( s_direction == 4 and (!(PINC & 0x02) or !(PINC & 0x04) or !(PINC & 0x08)))
  {
    s_direction = 3;
  }
}


ISR(PCINT0_vect)
{
   delay(50);
  
  if(!(PINB & 0x10) and s_direction!=4 and s_direction!=5) // A0, PC0,  button up
  {
    //snake_head.row -= 1;
    if(snake[0].row != snake[1].row + 1) // direction of snake 0=up; 1=down; 2=left; 3=right
    {
      s_direction = 0; // direction of snake 0=up; 
    }
  }
  else if( s_direction == 4 and !(PINB & 0x10))
  {
    s_direction = 3;
  }
}
