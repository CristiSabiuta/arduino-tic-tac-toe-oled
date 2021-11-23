#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#define ZERO 0
#define X    1
#define NONE 2
#define DRAW 3
#define START_SCREEN 0 /* Welcome screen */
#define MENU 1         /* Choose between (1 vs 1) and (1 vs CPU) */
#define GAME 2
#define END_SCREEN 3
#define GAME_MODE_1V1 0
#define GAME_MODE_1VCPU 1
#define PLAYER 0
#define CPU 1

struct box_t{
  uint8_t x;
  uint8_t y;
};

void draw_board(void);
void draw_info(void);
void draw_x(box_t box_id);
void draw_zero(box_t box_id);
void highlight_box(box_t box_id);
void clear_board(void);
void make_move_zero(uint8_t board_index);
void make_move_x(uint8_t board_index);
void increment_cursor(void);
bool check_box(uint8_t board_index);
void print_board_matrix(void);
void print_results(uint8_t result);
uint8_t check_game_result(void);
void check_button_press(uint8_t page);
void print_screen(uint8_t page);
void reset_game(void);
void get_random_game(void);
void gameplay_1vscpu(void);

Adafruit_SSD1306 display(128, 64);
const int cursorButton = 8;
const int selectButton = 9;
const int randomSeedGenerator = A0;

struct box_t board[] = {{64,0}, {84,0}, {104,0}, {64,20}, {84,20}, {104,20}, {64,40}, {84,40}, {104,40}};
uint8_t board_matrix[3][3];
uint8_t game_state = START_SCREEN;
uint8_t game_mode = GAME_MODE_1V1;
uint8_t current_player, cpu;
uint8_t current_box = 0;
uint8_t game_result = NONE;
uint8_t randomizer = 0, cpu_randomizer = 0;
uint8_t turn;
uint8_t move_done = 0;

void setup() 
{
  delay(100);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setRotation(0);
  display.setTextWrap(false);
  display.dim(0);
  
  pinMode(cursorButton, INPUT);
  pinMode(selectButton, INPUT);

  randomSeed(analogRead(randomSeedGenerator));
  
  clear_board();
}

void loop() 
{
  if(game_state == START_SCREEN)
  {
    print_screen(game_state);
    check_buttons(game_state);
  }
   
  if(game_state == MENU)
  {
    print_screen(game_state);
    check_buttons(game_state);
  }

  if(game_state == GAME)
  {
    if(game_mode == GAME_MODE_1V1)
    {
      get_random_game();
      print_screen(game_state);
      check_buttons(game_state);
      game_result = check_game_result();
    }
    if(game_mode == GAME_MODE_1VCPU)
    {
      get_random_game();
      draw_board();
      draw_info();
      gameplay_1vscpu();
      print_board_matrix();
      game_result = check_game_result();
      display.display();
    }
  }

  if(game_state == END_SCREEN)
  {
    print_screen(game_state);
    reset_game(); 
    check_buttons(game_state);
  }
}

void draw_board(void)
{
  display.clearDisplay();
  display.drawRect(84, 2, 2, 58, WHITE);
  display.drawRect(104, 2, 2, 58, WHITE);
  display.drawRect(66, 20, 58, 2, WHITE);
  display.drawRect(66, 40, 58, 2, WHITE);
}

void draw_info(void)
{
  display.setFont();
  highlight_box({21,15});
  if(current_player == X)
  {
    draw_x({21,15});
  }
  else
  {
    draw_zero({21,15});
  }
  display.setCursor(12,38);
  display.println("to play");
}

void draw_x(box_t box_id)
{
  display.drawLine(box_id.x + 6, box_id.y + 6, box_id.x + 16, box_id.y + 16, WHITE);
  display.drawLine(box_id.x + 6, box_id.y + 16, box_id.x + 16, box_id.y + 6, WHITE);
}

void draw_zero(box_t box_id)
{
  display.drawCircle(box_id.x + 11, box_id.y + 11, 5, WHITE);
}

void highlight_box(box_t box_id)
{
  display.drawRoundRect(box_id.x + 3, box_id.y + 3, 16, 16, 2, WHITE);
}

void clear_board(void)
{
  for(uint8_t i = 0; i < 3; i++)
  {
    for(uint8_t j = 0; j < 3; j++)
    {
      board_matrix[i][j] = NONE;
    }
  }
}

void make_move_zero(uint8_t board_index)
{
  board_matrix[board_index/3][board_index%3] = ZERO;
}

void make_move_x(uint8_t board_index)
{
  board_matrix[board_index/3][board_index%3] = X;
}

void increment_cursor(void)
{
  uint8_t timeout = 0;
  
  current_box++;
  if(current_box == 9)
  {
    current_box = 0;
  }
  while((check_box(current_box) == false) && (timeout < 10))
  {
    current_box++;
    if(current_box == 9)
    {
      current_box = 0;
    }
    timeout++;
  }
}

bool check_box(uint8_t board_index)
{
  if(board_matrix[board_index/3][board_index%3] == NONE)
    return true;
  else  
    return false;
}

void print_board_matrix(void)
{
  for(uint8_t i = 0; i < 3; i++)
  {
    for(uint8_t j = 0; j < 3; j++)
    {
      if(board_matrix[i][j] == ZERO)
      {
        draw_zero(board[i * 3 + j]);
      }
      if(board_matrix[i][j] == X)
      {
        draw_x(board[i * 3 + j]);
      }
    }
  }
}

uint8_t check_game_result(void)
{
  /* Check 1st line */
  if(board_matrix[0][0] == board_matrix[0][1])
    if(board_matrix[0][1] == board_matrix[0][2])
      if(board_matrix[0][0] != NONE)
      {
        game_state = END_SCREEN;
        return board_matrix[0][0];
      }
      
  /* Check 2nd line */
  if(board_matrix[1][0] == board_matrix[1][1])
    if(board_matrix[1][1] == board_matrix[1][2])
      if(board_matrix[1][0] != NONE)
      {
        game_state = END_SCREEN;
        return board_matrix[1][0];
      }
  
  /* Check 3rd line */
  if(board_matrix[2][0] == board_matrix[2][1])
    if(board_matrix[2][1] == board_matrix[2][2])
      if(board_matrix[2][0] != NONE)
      {
        game_state = END_SCREEN;
        return board_matrix[2][0];
      }
      
  /* Check 1st column */
  if(board_matrix[0][0] == board_matrix[1][0])
    if(board_matrix[1][0] == board_matrix[2][0])
      if(board_matrix[0][0] != NONE)
      {
        game_state = END_SCREEN;
        return board_matrix[0][0];
      }
  
  /* Check 2nd column */
  if(board_matrix[0][1] == board_matrix[1][1])
    if(board_matrix[1][1] == board_matrix[2][1])
      if(board_matrix[0][1] != NONE)
      {
        game_state = END_SCREEN;
        return board_matrix[0][1];
      }
      
  /* Check 3rd column */
  if(board_matrix[0][2] == board_matrix[1][2])
    if(board_matrix[1][2] == board_matrix[2][2])
      if(board_matrix[0][2] != NONE)
      {
        game_state = END_SCREEN;
        return board_matrix[0][2];
      }
      
  /* Check 1st diag */
  if(board_matrix[0][0] == board_matrix[1][1])
    if(board_matrix[1][1] == board_matrix[2][2])
      if(board_matrix[0][0] != NONE)
      {
        game_state = END_SCREEN;
        return board_matrix[0][0];
      }
      
  /* Check 2nd diag */
  if(board_matrix[0][2] == board_matrix[1][1])
    if(board_matrix[1][1] == board_matrix[2][0])
      if(board_matrix[0][2] != NONE)
      {
        game_state = END_SCREEN;
        return board_matrix[0][2];
      }

  /* Check if the board is fully completed */
  uint8_t board_full = 1;
  for(uint8_t i = 0; i < 3; i++)
    for(uint8_t j = 0; j < 3; j++)
      if(board_matrix[i][j] == NONE)
      {
        board_full = 0;
      }

  if(board_full == 1)
  {
    game_state = END_SCREEN;
    return DRAW;
  }

  return NONE;
}

void check_buttons(uint8_t page)
{
  if(page == START_SCREEN)
  {
    if (digitalRead(cursorButton) == LOW)
    {
      delay(200);
      game_state = MENU;
      while(digitalRead(cursorButton) != HIGH);
    }
      
    if (digitalRead(selectButton) == LOW)
    {
      delay(200);
      game_state = MENU;
      while(digitalRead(selectButton) != HIGH);
    }
  }

  if(page == MENU)
  {
    if (digitalRead(cursorButton) == LOW)
    {
      delay(200);
      if(game_mode == GAME_MODE_1V1)
      {
        game_mode = GAME_MODE_1VCPU;
      }
      else
      {
        game_mode = GAME_MODE_1V1;
      }
      while(digitalRead(cursorButton) != HIGH);
    }

    if (digitalRead(selectButton) == LOW)
    {
      delay(200);
      game_state = GAME;
      while(digitalRead(cursorButton) != HIGH);
    }
  }

  if(page == GAME)
  {
    if(game_mode == GAME_MODE_1V1)
    {
      if (digitalRead(cursorButton) == LOW)
      {
        delay(200);
        increment_cursor();
        while(digitalRead(cursorButton) != HIGH);
      }
      
      if (digitalRead(selectButton) == LOW)
      {
        delay(200);
        if(current_player == X)
        {
          make_move_x(current_box);
          current_player = ZERO;
          increment_cursor();
        }
        else
        {
          make_move_zero(current_box);
          current_player = X;
          increment_cursor();
        }
        while(digitalRead(selectButton) != HIGH);
       }
    }
    
    if(game_mode == GAME_MODE_1VCPU)
    {
      if (digitalRead(cursorButton) == LOW)
      {
        delay(200);
        increment_cursor();
        while(digitalRead(cursorButton) != HIGH);
      }
      
      if (digitalRead(selectButton) == LOW)
      {
        delay(200);
        if(current_player == X)
        {
          make_move_x(current_box);
          increment_cursor();
          move_done = 1;
        }
        else
        {
          make_move_zero(current_box);
          increment_cursor();
          move_done = 1;
        }
        while(digitalRead(selectButton) != HIGH);
       }
    }
  }

  if(page == END_SCREEN)
  {
    if (digitalRead(cursorButton) == LOW)
    {
      delay(200);
      game_state = START_SCREEN;
      while(digitalRead(cursorButton) != HIGH);
    }
    
    if (digitalRead(selectButton) == LOW)
    {
      delay(200);
      game_state = START_SCREEN;
      while(digitalRead(selectButton) != HIGH);
    }
  }
}

void print_screen(uint8_t page)
{
  if(page == START_SCREEN)
  {
    display.clearDisplay();
    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(4,20);
    display.println("TIC-TAC-TOE");
    display.setFont();
    display.setCursor(16,40);
    display.println("Press any button");
    display.display();
  }

  if(page == MENU)
  {
    display.clearDisplay();
    display.setFont(&FreeSansBold9pt7b);
    display.drawRoundRect(4, 8, 21, 21, 3, WHITE);
    display.drawRoundRect(4, 36, 21, 21, 3, WHITE);
    display.setCursor(32,24);
    display.println("1 VS 1");
    display.setCursor(32,52);
    display.println("1 VS CPU");
    
    if(game_mode == GAME_MODE_1V1)
    {
      display.fillCircle(14, 18, 6, WHITE);
    }
    if(game_mode == GAME_MODE_1VCPU)
    {
      display.fillCircle(14, 46, 6, WHITE);
    }

    display.display();
  }

  if(page == GAME)
  {
    if(game_mode == GAME_MODE_1V1)
    {
      draw_board();
      draw_info();
      highlight_box(board[current_box]);
      print_board_matrix();

      display.display();
    }

    if(game_mode == GAME_MODE_1VCPU)
    {
      
    }
  }

  if(page == END_SCREEN)
  {
    display.clearDisplay();
    
    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(31,38);
    if(game_result == X)
    {
      display.println("X WINS");
    }
    else if(game_result == ZERO)
    {
      display.println("0 WINS");
    }
    else if(game_result == DRAW)
    {
      display.println("  DRAW");
    } 

    display.display();
  }
}

void reset_game(void)
{
  clear_board();
  randomizer = 0;
  current_box = 0;
}

void get_random_game(void)
{
  if(game_mode == GAME_MODE_1V1)
  {
    if(randomizer == 0)
      for(uint8_t index = 0; index < random(255); index++)
      {
        current_player = random(2);
        randomizer = 1;
      }
  }

  if(game_mode == GAME_MODE_1VCPU)
  {
    if(randomizer == 0)
    {
      for(uint8_t index = 0; index < random(255); index++)
      {
        current_player = random(2);
        turn = random(2);
        randomizer = 1;
      }

      if(current_player == X) cpu = ZERO;
      else cpu = X;
    }
  }
}

void gameplay_1vscpu(void)
{
  if(turn == PLAYER)
  {
    highlight_box(board[current_box]);
    move_done = 0;
    check_buttons(game_state);
    if(move_done == 1)
    {
      turn = CPU;
    }
  }
  else if(turn == CPU)
  {
    for(uint8_t index = 0; index < random(255); index++)
    {
      increment_cursor();
    }
    if(cpu == X)
    {
      make_move_x(current_box);
      increment_cursor();
    }
    else
    {
      make_move_zero(current_box);
      increment_cursor();
    }
    turn = PLAYER;
  }
}
