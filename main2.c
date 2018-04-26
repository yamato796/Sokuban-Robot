#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

float raw1[12][6];
int obj_num=0;
int origin_map[5][5];
int robot_map[5][5];
char robot_rout[1000];
int dot[3][2];
int strinc = 0;

//picture to map
void picture_to_map(void)
{
    int i,j,x_cur,y_cur;
    for(i=0;i<5;i++){
        for(j=0;j<5;j++){
            origin_map[i][j] = 0;
        }
    }
    float len,wide;
    FILE* fp = fopen("output1","r");
    fscanf(fp,"%f %f\n",&len,&wide);
    /* raw manual
     0: pixel size
     1: x coordinate
     2: y coordinate
     3: orientation angle of object
     4: 1=destination or 2=blocks
     5: colours or 0=obsticle
     */
    wide/=5;
    len /=5;
    while(fscanf(fp,"%f %f %f %f %f %f\n",&raw1[obj_num][0],&raw1[obj_num][1],&raw1[obj_num][2],&raw1[obj_num][3],&raw1[obj_num][4],&raw1[obj_num][5])!=EOF){
        x_cur =0;
        y_cur =0;
        while((y_cur*wide)<raw1[obj_num][1]){
            y_cur++;
        }
        while((x_cur*wide)<raw1[obj_num][2]){
            x_cur++;
        }
        if(raw1[obj_num][5]==0){
            origin_map[x_cur-1][y_cur-1] = 2;
            obj_num++;
        }else if(raw1[obj_num][5]==1){
            if(raw1[obj_num][4]==1){
                origin_map[x_cur-1][y_cur-1] = 6;
            }else if(raw1[obj_num][4]==2){
                origin_map[x_cur-1][y_cur-1] = 3;
            }
            obj_num++;
        }else if(raw1[obj_num][5]==2){
            if(raw1[obj_num][4]==1){
                origin_map[x_cur-1][y_cur-1] = 7;
            }else if(raw1[obj_num][4]==2){
                origin_map[x_cur-1][y_cur-1] = 4;
            }
            
            obj_num++;
        }else if(raw1[obj_num][5]==3){
            if(raw1[obj_num][4]==1){
                origin_map[x_cur-1][y_cur-1] = 8;
            }else if(raw1[obj_num][4]==2){
                origin_map[x_cur-1][y_cur-1] = 5;
            }
            obj_num++;
        }
        
    }
    //if((x < 0)||(x >= 5)||(y < 0)||(y >= 5))
    //return 1;
    //int m[5][5];
    /*int m[5][5] = {0, 0, 0, 0, 0,
     0, 2, 3, 0, 2,
     0, 6, 0, 8, 0,
     0, 5, 2, 0, 0,
     0, 0, 4, 0, 7};*/
    //return m[x][y];
    
}

int map(int x,int y, int map_index){
    if(x>4 || y>4 || x<0 || y<0){
        return 1;
    }
    
    if(map_index == 0)
        return origin_map[x][y];
    else if(map_index == 1)
        return robot_map[x][y];
}


//creat box_rout_map
int check_dirty_map(int x, int y, int orien, int *dirty_map)
{
    /*
     int flag;
     if((orien == 1)||(orien == 2))
     flag = 0;
     else if((orien == 3)||(orien == 4))
     flag = 1;
     */
    if(orien == 0)
        return -1;
    //printf("check dirty (%d, %d) with orien = %d\n", x, y, orien);
    return dirty_map[(orien-1)*5*5 + x*5 + y];
}

void set_dirty_map(int x, int y, int orien, int *dirty_map, int value)
{
    /*
     int flag;
     if((orien == 1)||(orien == 2))
     flag = 0;
     else if((orien == 3)||(orien == 4))
     flag = 1;
     */
    
    dirty_map[(orien-1)*5*5 + x*5 + y] = value;
}

void save_rout(int x, int y, int rout, int* rout_map) //0: X, 1~4: available rout, need to move box1, 5~8: available rout, need to move box2, 9~12: available rout, need to move box3
{
    *(rout_map + (x*5 + y)) = rout;
    //printf("save %d, %d as %d\n", x, y, rout_map[x*5 + y]);
    return;
}

int temp[5][5];
int go(int x, int y, int orien, int *rout_map, int *dirty_map, int number, int map_index)
{
    //printf("enter go from (%d, %d), orien = %d\n", x, y, orien);
    int object, rout = 0;
    
    if((x < 0)||(x >= 5)||(y < 0)||(y >= 5))
        return 0;
    
    if(temp[x][y] == 1){
        temp[x][y]=0;
        //printf("rout %d, %d = %d temp\n",x, y, 0 );
        return 0;
    }else{
        temp[x][y]=1;
    }
    int dirty_value =  check_dirty_map(x, y, orien, dirty_map);
    if(dirty_value != -1){
        //printf("dirty!! (%d, %d) is %d\n", x, y, dirty_value);
        temp[x][y]=0;
        
        return dirty_value;
    }
    
    
    object = map(x, y, map_index);
    if((object == 1) || (object == 2)){
        rout = 0;
        //printf("rout %d, %d = %d obstacle\n", x, y, rout);
        set_dirty_map(x, y, orien, dirty_map, rout);
        temp[x][y]=0;
        return rout;
    }
    
    if(orien){
        int available_result = available(x, y, map_index);
        //printf("available result = %d, orien = %d\n", available_result, orien);
        if(available_result != 3){
            if((orien == 1)||(orien == 2)){
                if(available_result != 1){
                    rout = 0;
                    //printf("rout %d, %d = %d orien\n", x, y, rout);
                    set_dirty_map(x, y, orien, dirty_map, rout);
                    temp[x][y]=0;
                    return rout;
                }
            }
            
            else if((orien == 3)||(orien == 4)){
                if(available_result != 2){
                    rout = 0;
                    //printf("rout %d, %d = %d orien\n", x, y, rout);
                    set_dirty_map(x, y, orien, dirty_map, rout);
                    temp[x][y]=0;
                    return rout;
                }
            }
        }
    }
    
    if(object == 3){
        rout += 1;
        if(number == 1){
            save_rout(x, y, rout, rout_map);
            set_dirty_map(x, y, orien, dirty_map, rout);
            //printf("rout %d, %d = %d box1\n", x, y, rout);
            temp[x][y]=0;
            return rout;
        }
    }
    else if(object == 4){
        rout += 5;
        if(number == 2){
            save_rout(x, y, rout, rout_map);
            set_dirty_map(x, y, orien, dirty_map, rout);
            //printf("rout %d, %d = %d box2\n", x, y, rout);
            temp[x][y]=0;
            return rout;
        }
    }
    else if(object == 5){
        rout += 9;
        if(number == 3){
            save_rout(x, y, rout, rout_map);
            set_dirty_map(x, y, orien, dirty_map, rout);
            //printf("rout %d, %d = %d box3\n", x, y, rout);
            temp[x][y]=0;
            return rout;
        }
    }
    
    //0: X, 1: L, 2: R, 3: U, 4: D
    switch(orien){
        case 1:
            rout += go(x, y+1, 1, rout_map, dirty_map, number, map_index) + go(x+1, y, 3, rout_map, dirty_map, number, map_index) + go(x-1, y, 4, rout_map, dirty_map, number, map_index);
            break;
        case 2:
            rout += go(x, y-1, 2, rout_map, dirty_map, number, map_index) + go(x+1, y, 3, rout_map, dirty_map, number, map_index) + go(x-1, y, 4, rout_map, dirty_map, number, map_index);
            break;
        case 3:
            rout += go(x+1, y, 3, rout_map, dirty_map, number, map_index) + go(x, y-1, 2, rout_map, dirty_map, number, map_index) + go(x, y+1, 1, rout_map, dirty_map, number, map_index);
            break;
        case 4:
            rout += go(x-1, y, 4, rout_map, dirty_map, number, map_index) + go(x, y-1, 2, rout_map, dirty_map, number, map_index) + go(x, y+1, 1, rout_map, dirty_map, number, map_index);
            break;
        default:
            rout += go(x+1, y, 3, rout_map, dirty_map, number, map_index) + go(x-1, y, 4, rout_map, dirty_map, number, map_index) + go(x, y+1, 1, rout_map, dirty_map, number, map_index) + go(x, y-1, 2, rout_map, dirty_map, number, map_index);
            break;
    }
    
    int flag = 0;
    switch(number){
        case 1:{
            if((rout == 1)||(rout == 6)||(rout == 10)||(rout > 14))
                flag = 1;
            break;
        }
        case 2:{
            if((rout == 5)||(rout == 6)||(rout == 14)||(rout > 14))
                flag = 1;
            break;
        }
        case 3:{
            if((rout == 9)||(rout == 10)||(rout == 14)||(rout > 14))
                flag = 1;
            break;
        }
        default :
            flag = 0;
            break;
    }
    
    if(flag == 0)
        rout = 0;
    save_rout(x, y, rout, rout_map);
    set_dirty_map(x, y, orien, dirty_map, rout);
    //printf("rout %d, %d = %d return\n", x, y, rout);
    temp[x][y]=0;
    return rout;
}


int initial_rout_map(int *rout_map) // initial to 0
{
    int i = 0;
    for(i = 0; i < 3*5*5; i++)
        rout_map[i] = 0;
    
    return i;
}

int initial_dirty_map(int *dirty_map) //initial to -1
{
    int i = 0;
    for(i = 0; i < 4*5*5; i++)
        dirty_map[i] = -1;
    
    return i;
}

int available(int x, int y, int map_index) //0: X, 1: LR, 2: UD, 3:LRUD
{
    if((x < 0)||(x >= 5)||(y < 0)||(y >= 5))
        return 0;
    
    int flag = 0;
    if((map(x, y-1, map_index) != 2)&&(map(x, y-1, map_index) != 1)&&(map(x, y+1, map_index) != 2)&&(map(x, y+1, map_index) != 1))
        flag = 1;
    if((map(x-1, y, map_index) != 1)&&(map(x-1, y, map_index) != 2)&&(map(x+1, y, map_index) != 1)&&(map(x+1, y, map_index) != 2)){
        if(flag)
            return 3;
        else
            return 2;
    }
    return flag;
}

void print_map(int *map)
{
    int i, j;
    printf("this is map :\n");
    for(i = 0; i < 5; i++){
        for(j = 0; j < 5; j++)
            printf(" %d ", map[i*5 + j]);
        printf("\n");
    }
    printf("\n");
}

void print_rout_map(int *rout_map)
{
    int i, j, k;
    for(i = 0; i < 3; i++){
        printf("this is No.%d rout map\n", i+1);
        for(j = 0; j < 5; j++){
            for(k = i*5*5 + j*5; k < i*5*5+j*5+5; k++)
                printf(" %d ", rout_map[k]);
            printf("\n");
        }
        printf("\n\n");
    }
    return;
}



//map decoder

int check_rout_value_map(int x, int y, int *rout_value_map)
{
    if((x < 0)||(x >= 5)||(y < 0)||(y >= 5))
        return 0;
    
    return rout_value_map[x*5 + y];
}

int check_rout_value(int rout_value, int number)
{
    if(rout_value == 0)
        return 0;
    
    int flag;
    switch(number){
        case 1:{
            if((rout_value == 1)||(rout_value == 6)||(rout_value == 10)||(rout_value > 14))
                flag = 1;
            break;
        }
        case 2:{
            if((rout_value == 5)||(rout_value == 6)||(rout_value == 14)||(rout_value > 14))
                flag = 1;
            break;
        }
        case 3:{
            if((rout_value == 9)||(rout_value == 10)||(rout_value == 14)||(rout_value > 14))
                flag = 1;
            break;
        }
        default :
            flag = 0;
            break;
    }
    
    return flag;
}


void initial_robot_map(void)
{
    int i, j;
    for(i = 0; i < 5; i++)
        for(j = 0; j < 5; j++){
            if((map(i, j, 0) == 0)||(map(i, j, 0) >= 6))
                robot_map[i][j] = 0; //set dot as free as well
            else
                robot_map[i][j] = 2; //set box as obstacle as well
        }
}

void set_robot_map(int x, int y, int value)
{
    //printf("set %d, %d as %d\n", x, y, value);
    //print_map(&robot_map[0][0]);
    
    if((x >= 0)&&(x < 5)&&(y >= 0)&&(y < 5))
        robot_map[x][y] = value;
}

//0: X, 1: L, 2: R, 3: U, 4: D
void robot_map_decoder(int now_x, int now_y, int go_x, int go_y)
{
    char buffer;
    if(go_x > now_x){
        if(robot_map[now_x+1][now_y]){
            //do down
            buffer = 'd';
            strcat(robot_rout, &buffer);
            
            robot_map_decoder(now_x+1, now_y, go_x, go_y);
        }
    }
    
    else{
        if(robot_map[now_x-1][now_y]){
            //do up
            buffer = 'u';
            strcat(robot_rout, &buffer);
            
            robot_map_decoder(now_x-1, now_y, go_x, go_y);
        }
    }
    
    if(go_y > now_y){
        if(robot_map[now_x][now_y+1]){
            //do right
            buffer = 'r';
            strcat(robot_rout, &buffer);
            
            robot_map_decoder(now_x, now_y+1, go_x, go_y);
        }
    }
    
    else{
        if(robot_map[now_x-1][now_y]){
            //do left
            buffer = 'l';
            strcat(robot_rout, &buffer);
            
            robot_map_decoder(now_x, now_y-1, go_x, go_y);
        }
    }
}

int initial_robot_to(int x, int y, int number)
{
    //printf("initial NO.%d map '%c' at %d, %d\n", number, robot_rout[strinc-2], x, y);
    //int in;
    //for(in = 0; in < 3; in++)
    //printf("dot%d = %d, %d\n", in, dot[in][0], dot[in][1]);
    
    switch(robot_rout[strinc-2]){
        case 'l':
            robot_go_to(dot[number-2][0], dot[number-2][1]+1, x, y, 0);
            break;
        case 'r':
            robot_go_to(dot[number-2][0], dot[number-2][1]-1, x, y, 0);
            break;
        case 'u':
            robot_go_to(dot[number-2][0]+1, dot[number-2][1], x, y, 0);
            break;
        case 'd':
            robot_go_to(dot[number-2][0]-1, dot[number-2][1], x, y, 0);
            break;
        default :
            robot_go_to(4, 4, x, y, 0);
            break;
    }
    
    int i = robot_rout[strinc-1];
    if(i == 'l')
        return 1;
    else if(i == 'r')
        return 2;
    else if(i == 'u')
        return 3;
    else if(i == 'd')
        return 4;
    else
        return i;
}

//0: X, 1: V, 2: you are here
/*
 int robot_go_to(int now_x, int now_y, int go_x, int go_y)
 {
	printf("robot go from %d, %d to %d, %d\n", now_x, now_y, go_x, go_y);
	if((now_x == go_x)&&(now_y == go_y))
 return 2;
	
	int robot_dirty_map[4][5][5];
	int flag;
	flag = initial_dirty_map(&robot_dirty_map[0][0][0]);
	if(flag == 0)
 printf("initial dirty map erro\n");
	
	robot_map[now_x][now_y] = 3;
	
	int robot_rout_map[5][5];
	int i, j;
	for(i = 0; i < 5; i++)
 for(j = 0; j < 5; j++)
 robot_rout_map[i][j] = 0;
	
	flag = go(go_x, go_y, 0, &robot_rout_map[0][0], &robot_dirty_map[0][0][0], 1, 1);
	printf("this is robot rout map\n");
	print_map(&robot_rout_map[0][0]);
	
	
	if(flag)
 robot_map_decoder(now_x, now_y, go_x, go_y);
	
	robot_map[now_x][now_y] = 0;
	
	return flag;
 }
 */

int check_robot_map(int x, int y)
{
    if(x>4 || y>4 || x<0 || y<0){
        return 2;
    }
    
    return robot_map[x][y];
}
int robot_go_to(int x0, int y0, int x1, int y1, int orien)
{
    char buffer;
    
    //printf("robot go to %d, %d from %d, %d\n", x1, y1, x0, y0);
    
    if((x0 == x1)&&(y0 == y1))
        return 2;
    
    int flag  = 0;
    if((check_robot_map(x0+1, y0) == 0)&&(orien != 3))
        flag += 1;
    if((check_robot_map(x0-1, y0) == 0)&&(orien != 4))
        flag += 2;
    if((check_robot_map(x0, y0+1) == 0)&&(orien != 1))
        flag += 3;
    if((check_robot_map(x0, y0-1) == 0)&&(orien != 2))
        flag += 4;
    
    //printf("go_to flag = %d\n ", flag);
    if(flag == 1){
        //if(check_robot_map(x0+1, y0) == 0){
        buffer = 'd';
        //strcat(robot_rout, &buffer);
        robot_rout[strinc] = buffer;
        strinc++;
        //return robot_go_to(x0+1, y0, x1, y1, 4);
        //}
    }
    
    else if(flag == 2){
        //if(check_robot_map(x0-1, y0) == 0){
        buffer = 'u';
        //strcat(robot_rout, &buffer);
        robot_rout[strinc] = buffer;
        strinc++;
        return robot_go_to(x0-1, y0, x1, y1, 3);
        //}
    }
    
    if(flag == 3){
        //if(check_robot_map(x0, y0+1) == 0){
        buffer = 'r';
        //strcat(robot_rout, &buffer);
        robot_rout[strinc] = buffer;
        strinc++;
        return robot_go_to(x0, y0+1, x1, y1, 2);
        //}
    }
    
    else if(flag == 4){
        //if(check_robot_map(x0, y0-1) == 0){
        buffer = 'l';
        //strcat(robot_rout, &buffer);
        robot_rout[strinc] = buffer;
        strinc++;
        return robot_go_to(x0, y0-1, x1, y1, 1);
        //}
    }
    
    else{
        if(x1 > x0){
            if((check_robot_map(x0+1, y0) == 0)&&(orien != 3)){
                
                buffer = 'd';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
                return robot_go_to(x0+1, y0, x1, y1, 4);
            }
        }
        
        else {
            if((check_robot_map(x0-1, y0) == 0)&&(orien != 4)){
                buffer = 'u';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
                return robot_go_to(x0-1, y0, x1, y1, 3);
            }
        }
        
        if(y1 > y0){
            if((check_robot_map(x0, y0+1) == 0)&&(orien != 1)){
                buffer = 'r';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
                return robot_go_to(x0, y0+1, x1, y1, 2);
            }
        }
        
        else {
            if((check_robot_map(x0, y0-1) == 0)&&(orien != 2)){
                buffer = 'l';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
                return robot_go_to(x0, y0-1, x1, y1, 1);
            }
        }
    }
    //not idle
    
    /*
     {
     if(robot_map[x0-1][y0] == 0){
     buffer = 'u';
     //strcat(robot_rout, &buffer);
     robot_rout[strinc] = buffer;
     strinc++;
     return robot_go_to(x0-1, y0, x1, y1);
     }
     }
     
     
     {
     if(robot_map[x0][y0-1] == 0){
     buffer = 'l';
     //strcat(robot_rout, &buffer);
     robot_rout[strinc] = buffer;
     strinc++;
     return robot_go_to(x0, y0-1, x1, y1);
     }
     }
     */
    return 0;
}

//0: X, 1: L, 2: R, 3: U, 4: D, 5: grab, 6: open
int rout_value_map_decoder(int x, int y, int orien, int *dot, int number, int *rout_value_map) //dot (x, y) = (dot[0], dot[y])
{
    //printf("enter rout map decorder on %d, %d to %d, %d\nstring = ", x, y, dot[0], dot[1]);
    //print_string(robot_rout, strlen(robot_rout));
    //printf("\n");
    
    int rout_value;
    char buffer, turn;
    int robot[2];
    
    switch(orien){
        case 1:{
            robot[0] = x;
            robot[1] = y+1;
            break;
        }
        case 2:{
            robot[0] = x;
            robot[1] = y-1;
            break;
        }
        case 3:{
            robot[0] = x+1;
            robot[1] = y;
            break;
        }
        case 4:{
            robot[0] = x-1;
            robot[1] = y;
            break;
        }
    }
    
    int robot_flag = 0, orien_flag = 0;
    
    if((x == dot[0])&&(y == dot[1])){
        buffer = 'o';
        //strcat(robot_rout, &buffer);
        robot_rout[strinc] = buffer;
        strinc++;
        //printf("arrive\n");
        return 1;
    }
    
    int flag = 0;
    if((check_rout_value_map(x+1, y, rout_value_map))&&(orien != 3))
        flag += 1;
    else if((check_rout_value_map(x-1, y, rout_value_map))&&(orien != 4))
        flag += 2;
    else if((check_rout_value_map(x, y+1, rout_value_map))&&(orien != 1))
        flag += 3;
    else if((check_rout_value_map(x, y-1, rout_value_map))&&(orien != 2))
        flag += 4;
    
    if(flag == 1){
        rout_value = check_rout_value_map(x+1, y, rout_value_map);
        
        if(check_rout_value(rout_value, number)){
            if(orien != 4)
                orien_flag = 1;
            
            if(orien == 0){
                initial_robot_to(x-1, y, number);
                robot[0] = x-1;
                robot[1] = y;
                
                turn = robot_rout[strinc-1];
                if(turn == 'l')
                    buffer = '2';
                else if(turn == 'r')
                    buffer = '4';
                else if(turn == 'u')
                    buffer = '8';
                else if(turn == 'd')
                    buffer = '6';
                //buffer = 'g';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
            }
            
            if(orien_flag && orien){
                buffer = 'o';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
                //open if need to turn
            }
            
            robot_flag = robot_go_to(robot[0], robot[1], x-1, y, 0); //turn to box
            if(robot_flag == 0)
                printf("robot can't go\n");
            
            if((orien_flag==1)||(orien == 0)){
                turn = robot_rout[strinc-1];
                if(turn == 'l')
                    buffer = '2';
                else if(turn == 'r')
                    buffer = '4';
                else if(turn == 'u')
                    buffer = '8';
                else if(turn == 'd')
                    buffer = '6';
                //buffer = 'g';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
                //grab if need to turn
            }
            
            buffer = 'd';
            //strcat(robot_rout, &buffer);
            robot_rout[strinc] = buffer;
            strinc++;
            set_robot_map(x, y, 0);
            set_robot_map(x+1, y, 2);
            
            //go down
            return (rout_value_map_decoder(x+1, y, 4, dot, number, rout_value_map));
        }
    }
    else if(flag == 2){
        rout_value = check_rout_value_map(x-1, y, rout_value_map);
        
        if(check_rout_value(rout_value, number)){
            if(orien != 3)
                orien_flag = 1;
            
            robot_rout[strinc] = buffer;
            strinc++;
            if(orien == 0){
                initial_robot_to(x+1, y, number);
                robot[0] = x+1;
                robot[1] = y;
                turn = robot_rout[strinc-1];
                if(turn == 'l')
                    buffer = '2';
                else if(turn == 'r')
                    buffer = '4';
                else if(turn == 'u')
                    buffer = '8';
                else if(turn == 'd')
                    buffer = '6';
                //buffer = 'g';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
            }
            
            if(orien_flag&&orien){
                buffer = 'o';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
                //open if need to turn
            }
            
            robot_flag = robot_go_to(robot[0], robot[1], x+1, y, 0); //turn to box
            if(robot_flag == 0)
                printf("robot can't go\n");
            
            if(orien_flag||(orien == 0)){
                turn = robot_rout[strinc-1];
                if(turn == 'l')
                    buffer = '4';
                else if(turn == 'r')
                    buffer = '2';
                else if(turn == 'u')
                    buffer = '6';
                else if(turn == 'd')
                    buffer = '8';
                //buffer = 'g';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
                //grab if need to turn
            }
            
            buffer = 'u';
            //strcat(robot_rout, &buffer);
            robot_rout[strinc] = buffer;
            strinc++;
            set_robot_map(x, y, 0);
            set_robot_map(x-1, y, 2);
            
            //go up
            return (rout_value_map_decoder(x-1, y, 3, dot, number, rout_value_map));
        }
    }
    
    else if(flag == 3){
        rout_value = check_rout_value_map(x, y+1, rout_value_map);
        
        if(check_rout_value(rout_value, number)){
            if(orien != 2)
                orien_flag = 1;
            
            if(orien == 0){
                initial_robot_to(x, y-1, number);
                robot[0] = x;
                robot[1] = y-1;
                turn = robot_rout[strinc-1];
                if(turn == 'l')
                    buffer = '2';
                else if(turn == 'r')
                    buffer = '4';
                else if(turn == 'u')
                    buffer = '8';
                else if(turn == 'd')
                    buffer = '6';
                //buffer = 'g';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
            }
            
            if(orien_flag&&orien){
                buffer = 'o';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
                //open if need to turn
            }
            
            robot_flag = robot_go_to(robot[0], robot[1], x, y-1, 0); //turn to box
            if(robot_flag == 0)
                printf("robot can't go\n");
            
            if(orien_flag||(orien == 0)){
                turn = robot_rout[strinc-1];
                if(turn == 'l')
                    buffer = '8';
                else if(turn == 'r')
                    buffer = '6';
                else if(turn == 'u')
                    buffer = '4';
                else if(turn == 'd')
                    buffer = '2';
                //buffer = 'g';
                //strcat(robot_rout, &buffer);
                
                robot_rout[strinc] = buffer;
                strinc++;
                //grab if need to turn
            }
            
            buffer = 'r';
            //strcat(robot_rout, &buffer);
            robot_rout[strinc] = buffer;
            strinc++;
            set_robot_map(x, y, 0);
            set_robot_map(x, y+1, 2);
            
            //go right
            return (rout_value_map_decoder(x, y+1, 2, dot, number, rout_value_map));
        }
    }
    
    else if(flag == 4){
        rout_value = check_rout_value_map(x, y-1, rout_value_map);
        
        if(check_rout_value(rout_value, number)){
            if(orien != 1)
                orien_flag = 1;
            
            if(orien == 0){
                initial_robot_to(x, y+1, number);
                robot[0] = x;
                robot[1] = y+1;
                turn = robot_rout[strinc-1];
                if(turn == 'l')
                    buffer = '2';
                else if(turn == 'r')
                    buffer = '4';
                else if(turn == 'u')
                    buffer = '8';
                else if(turn == 'd')
                    buffer = '6';
                //buffer = 'g';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
            }
            
            if(orien_flag&&orien){
                buffer = 'o';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
                //open if need to turn
            }
            
            robot_flag = robot_go_to(robot[0], robot[1], x, y+1, 0); //turn to box
            if(robot_flag == 0)
                printf("robot can't go\n");
            
            if(orien_flag||(orien == 0)){
                turn = robot_rout[strinc-1];
                if(turn == 'l')
                    buffer = '6';
                else if(turn == 'r')
                    buffer = '8';
                else if(turn == 'u')
                    buffer = '2';
                else if(turn == 'd')
                    buffer = '4';
                //buffer = 'g';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
                //grab if need to turn
            }
            
            buffer = 'l';
            //strcat(robot_rout, &buffer);
            
            robot_rout[strinc] = buffer;
            strinc++;
            set_robot_map(x, y, 0);
            set_robot_map(x, y-1, 2);
            
            //go left
            return (rout_value_map_decoder(x, y-1, 1, dot, number, rout_value_map));
        }
    }
    
    else{
        if(dot[0] > x){
            rout_value = check_rout_value_map(x+1, y, rout_value_map);
            
            if(check_rout_value(rout_value, number)){
                if(orien != 4)
                    orien_flag = 1;
                
                if(orien == 0){
                    initial_robot_to(x-1, y, number);
                    robot[0] = x-1;
                    robot[1] = y;
                    turn = robot_rout[strinc-1];
                    if(turn == 'l')
                        buffer = '2';
                    else if(turn == 'r')
                        buffer = '4';
                    else if(turn == 'u')
                        buffer = '8';
                    else if(turn == 'd')
                        buffer = '6';
                    //buffer = 'g';
                    //strcat(robot_rout, &buffer);
                    robot_rout[strinc] = buffer;
                    strinc++;
                }
                
                if(orien_flag && orien){
                    buffer = 'o';
                    //strcat(robot_rout, &buffer);
                    robot_rout[strinc] = buffer;
                    strinc++;
                    //open if need to turn
                }
                
                robot_flag = robot_go_to(robot[0], robot[1], x-1, y, 0); //turn to box
                if(robot_flag == 0)
                    printf("robot can't go\n");
                
                if((orien_flag==1)||(orien == 0)){
                    turn = robot_rout[strinc-1];
                    if(turn == 'l')
                        buffer = '2';
                    else if(turn == 'r')
                        buffer = '4';
                    else if(turn == 'u')
                        buffer = '8';
                    else if(turn == 'd')
                        buffer = '6';
                    //buffer = 'g';
                    //strcat(robot_rout, &buffer);
                    robot_rout[strinc] = buffer;
                    strinc++;
                    //grab if need to turn
                }
                
                buffer = 'd';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
                set_robot_map(x, y, 0);
                set_robot_map(x+1, y, 2);
                
                //go down
                return (rout_value_map_decoder(x+1, y, 4, dot, number, rout_value_map));
            }
        }else if(dot[0] < x){
            rout_value = check_rout_value_map(x-1, y, rout_value_map);
            
            if(check_rout_value(rout_value, number)){
                if(orien != 3)
                    orien_flag = 1;
                
                robot_rout[strinc] = buffer;
                strinc++;
                if(orien == 0){
                    initial_robot_to(x+1, y, number);
                    robot[0] = x+1;
                    robot[1] = y;
                    turn = robot_rout[strinc-1];
                    if(turn == 'l')
                        buffer = '2';
                    else if(turn == 'r')
                        buffer = '4';
                    else if(turn == 'u')
                        buffer = '8';
                    else if(turn == 'd')
                        buffer = '6';
                    //buffer = 'g';				
                    //strcat(robot_rout, &buffer);
                    robot_rout[strinc] = buffer;
                    strinc++;
                }
                
                if(orien_flag&&orien){
                    buffer = 'o';
                    //strcat(robot_rout, &buffer);
                    robot_rout[strinc] = buffer;
                    strinc++;
                    //open if need to turn
                }
                
                robot_flag = robot_go_to(robot[0], robot[1], x+1, y, 0); //turn to box
                if(robot_flag == 0)
                    printf("robot can't go\n");
                
                if(orien_flag||(orien == 0)){
                    turn = robot_rout[strinc-1];
                    if(turn == 'l')
                        buffer = '4';
                    else if(turn == 'r')
                        buffer = '2';
                    else if(turn == 'u')
                        buffer = '6';
                    else if(turn == 'd')
                        buffer = '8';
                    //buffer = 'g';				
                    //strcat(robot_rout, &buffer);
                    robot_rout[strinc] = buffer;
                    strinc++;
                    //grab if need to turn
                }
                
                buffer = 'u';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
                set_robot_map(x, y, 0);
                set_robot_map(x-1, y, 2);
                
                //go up
                return (rout_value_map_decoder(x-1, y, 3, dot, number, rout_value_map));
            }
        }
        
        if(dot[1] > y){
            rout_value = check_rout_value_map(x, y+1, rout_value_map);
            
            if(check_rout_value(rout_value, number)){
                if(orien != 2)
                    orien_flag = 1;
                
                if(orien == 0){
                    initial_robot_to(x, y-1, number);
                    robot[0] = x;
                    robot[1] = y-1;
                    turn = robot_rout[strinc-1];
                    if(turn == 'l')
                        buffer = '2';
                    else if(turn == 'r')
                        buffer = '4';
                    else if(turn == 'u')
                        buffer = '8';
                    else if(turn == 'd')
                        buffer = '6';
                    //buffer = 'g';				
                    //strcat(robot_rout, &buffer);
                    robot_rout[strinc] = buffer;
                    strinc++;
                }
                
                if(orien_flag&&orien){
                    buffer = 'o';
                    //strcat(robot_rout, &buffer);
                    robot_rout[strinc] = buffer;
                    strinc++;
                    //open if need to turn
                }
                
                robot_flag = robot_go_to(robot[0], robot[1], x, y-1, 0); //turn to box
                if(robot_flag == 0)
                    printf("robot can't go\n");
                
                if(orien_flag||(orien == 0)){
                    turn = robot_rout[strinc-1];
                    if(turn == 'l')
                        buffer = '8';
                    else if(turn == 'r')
                        buffer = '6';
                    else if(turn == 'u')
                        buffer = '4';
                    else if(turn == 'd')
                        buffer = '2';
                    //buffer = 'g';				
                    //strcat(robot_rout, &buffer);
                    
                    robot_rout[strinc] = buffer;
                    strinc++;
                    //grab if need to turn
                }
                
                buffer = 'r';
                //strcat(robot_rout, &buffer);
                robot_rout[strinc] = buffer;
                strinc++;
                set_robot_map(x, y, 0);
                set_robot_map(x, y+1, 2);
                
                //go right
                return (rout_value_map_decoder(x, y+1, 2, dot, number, rout_value_map));
            }
        }
        
        else if(dot[1] < y){
            rout_value = check_rout_value_map(x, y-1, rout_value_map);
            
            if(check_rout_value(rout_value, number)){
                if(orien != 1)
                    orien_flag = 1;
                
                if(orien == 0){
                    initial_robot_to(x, y+1, number);
                    robot[0] = x;
                    robot[1] = y+1;
                    turn = robot_rout[strinc-1];
                    if(turn == 'l')
                        buffer = '2';
                    else if(turn == 'r')
                        buffer = '4';
                    else if(turn == 'u')
                        buffer = '8';
                    else if(turn == 'd')
                        buffer = '6';
                    //buffer = 'g';				
                    //strcat(robot_rout, &buffer);
                    robot_rout[strinc] = buffer;
                    strinc++;
                }
                
                if(orien_flag&&orien){
                    buffer = 'o';
                    //strcat(robot_rout, &buffer);
                    robot_rout[strinc] = buffer;
                    strinc++;
                    //open if need to turn
                }
                
                robot_flag = robot_go_to(robot[0], robot[1], x, y+1, 0); //turn to box
                if(robot_flag == 0)
                    printf("robot can't go\n");
                
                if(orien_flag||(orien == 0)){
                    turn = robot_rout[strinc-1];
                    if(turn == 'l')
                        buffer = '6';
                    else if(turn == 'r')
                        buffer = '8';
                    else if(turn == 'u')
                        buffer = '2';
                    else if(turn == 'd')
                        buffer = '4';
                    //buffer = 'g';				
                    //strcat(robot_rout, &buffer);
                    robot_rout[strinc] = buffer;
                    strinc++;
                    //grab if need to turn
                }
                
                buffer = 'l';
                //strcat(robot_rout, &buffer);
                
                robot_rout[strinc] = buffer;
                strinc++;
                set_robot_map(x, y, 0);
                set_robot_map(x, y-1, 2);
                
                //go left
                return (rout_value_map_decoder(x, y-1, 1, dot, number, rout_value_map));
            }
        }
    }
    
    return 0;
}


void print_string(char *cha, int strlen)
{
    int i;
    for(i = 0; i <strinc; i++){
        /*if(((cha[i]-'0') == 2)||((cha[i]-'0') == 4)||((cha[i]-'0') == 6)||((cha[i]-'0') == 8))
         printf("%d", cha[i]-'0');
         else*/
        printf("%c", cha[i]);
    }
}

// testbench below
int main(void)
{
    int i, j, k;
    int box[3][2];
    bzero(robot_rout,sizeof(robot_rout));
    picture_to_map();
    //print_map(&origin_map[0][0]);
    
    int test_map[5][5] = { 
        8, 5, 0, 0, 6,
        0, 2, 4, 0, 0,
        0, 7, 0, 0, 3,
        0, 0, 2, 0, 0,
        0, 0, 0, 0, 0
    };
    
    for(i = 0; i < 5; i++)
        for(j = 0; j < 5; j++)
            origin_map[i][j] = test_map[i][j];
    //print_map(&origin_map[0][0]);
    
    for(i = 0; i < 5; i++)
        for(j = 0; j < 5; j++){
            temp[i][j] = 0; //initial temp
            if(map(i, j, 0) == 3){
                box[0][0] = i;
                box[0][1] = j;
                //printf("box1 = %d, %d\n", i, j);
            }
            else if(map(i, j, 0) == 4){
                box[1][0] = i;
                box[1][1] = j;
                //printf("box2 = %d, %d\n", i, j);
            }
            else if(map(i, j, 0) == 5){
                box[2][0] = i;
                box[2][1] = j;
                //printf("box1 = %d, %d\n", i, j);
            }
            else if(map(i, j, 0) == 6){
                dot[0][0] = i;
                dot[0][1] = j;
                //printf("dot1 = %d, %d\n", i, j);
            }
            else if(map(i, j, 0) == 7){
                dot[1][0] = i;
                dot[1][1] = j;
                //printf("dot2 = %d, %d\n", i, j);
            }
            else if(map(i, j, 0) == 8){
                dot[2][0] = i;
                dot[2][1] = j;
                //printf("dot3 = %d, %d\n", i, j);
            }
        }
    
    //for(i = 0; i < 3; i++)
    //printf("dot%d = %d, %d\n", i, dot[i][0], dot[i][1]);
    
    int rout_map[3][5][5];
    int dirty_map[4][5][5];
    
    int flag;
    
    flag = initial_rout_map(&rout_map[0][0][0]);
    if(flag == 0)
        printf("initial rout map erro\n");
    
    /*
     for(i = 0; i < 4; i++)
     for(j = 0; j < 5; j++)
     for(k = 0; k < 5; k++)
     temp[i][j][k] = 0;
     */
    
    
    for(i = 0; i < 3; i++){
        
        flag = initial_dirty_map(&dirty_map[0][0][0]);
        
        if(flag == 0)
            printf("initial dirty map erro\n");
        
        go(dot[i][0], dot[i][1], 0, &rout_map[i][0][0], &dirty_map[0][0][0], i+1, 0);
        //printf("\nfinish %d go\n", i+1);
        //print_rout_map(&rout_map[0][0][0]);
    }
    
    //print_map(&origin_map[0][0]);
    //print_rout_map(&rout_map[0][0][0]);
    
    //decorder testbench below
    
    initial_robot_map();
    //print_map(&robot_map[0][0]);
    
    /*
     for(i = 0; i < 4; i++)
     for(j = 0; j < 5; j++)
     for(k = 0; k < 5; k++)
     temp[i][j][k] = 0;
     */
    
    //for(i = 0; i < 3; i++)
    //printf("dot%d = %d, %d\n", i, dot[i][0], dot[i][1]);
    
    for(i = 0; i < 3; i++){
        //printf("dot = %d, %d\n", dot[i][0], dot[i][1]);
        //print_map(&robot_map[0][0]);
        //for(j = 0; j < 3; j++)
        //printf("dot%d = %d, %d\n", j, dot[j][0], dot[j][1]);
        
        rout_value_map_decoder(box[i][0], box[i][1], 0, &dot[i][0], i+1, &rout_map[i][0][0]);
        
        //for(i = 0; i < 3; i++)
        //printf("dot%d = %d, %d\n", i, dot[i][0], dot[i][1]);
        //printf("this is NO.%d map\n", i+1);
        //print_map(&robot_map[0][0]);
    }
    
    //print_map(&origin_map[0][0]);
    //print_string(robot_rout, strlen(robot_rout));
    
    return 0;
}