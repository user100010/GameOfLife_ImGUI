//
// Created by
//          Benjamin Grothe
// on 06.05.23
// all Copyrights included by Benjamin Grothe
//

#include "game_of_life.h"
#include <cmath>
#include <utility>

namespace gol {
    //================================================================================================= CLASS CELL ===//
    Cell::Cell(Grid *_grid, Vec2 _position) {
        grid = _grid;
        position = _position;
        state = false;              // initialize Cell object as death
        color = IM_COL32_WHITE;     // initialize Cell color as white
    }
    //================================================================================================= CLASS GRID ===//
    Grid::Grid(GameOfLife *_gameOfLife, Vec2 _dimension) {
        gameOfLife = _gameOfLife;
        dimension = _dimension;

        // initialize Grid object with x * y death Cell objects
        for(int y = 0; y < dimension.y; y++){
            for(int x = 0; x < dimension.x; x++){
                cells.push_back(new Cell(this, (Vec2){x,y}));
            }
        }

        // create for each Cell object in Grid object a Neighbours object
        for(int y = 0; y < dimension.y; y++){
            for(int x = 0; x < dimension.x; x++){
                new Neighbours(cells[(y*dimension.x)+x]);
            }
        }
    }

    Vec2 Grid::get_real_grid_dimension() {
        int first_row  = 0; bool first_row_flag = false;
        int last_row   = 0;

        // filter first and last row
        for(auto c: cells){
            if(c->get_state()){
                if(!first_row_flag){
                    first_row_flag = true;
                    first_row = c->get_position().y;
                    last_row = c->get_position().y;
                }else{
                    last_row = c->get_position().y;
                }
            }
        }

        int first_column  = 0; bool first_column_flag = false;
        int last_column   = 0;
        // filter first and last column
        for(int x=0; x < dimension.x; x++){
            for(int y=0; y < dimension.y; y++){
                if(cells[(y*dimension.x)+x]->get_state()){
                    if(!first_column_flag){
                        first_column_flag = true;
                        first_column = cells[(y*dimension.x)+x]->get_position().x;
                        last_column = cells[(y*dimension.x)+x]->get_position().x;
                    }else{
                        last_column = cells[(y*dimension.x)+x]->get_position().x;
                    }
                }
            }
        }

        int y = last_row-first_row+1;
        int x = last_column-first_column+1;


        return Vec2{x,y};
    }
    float Grid::get_auto_zoom_factor() {
        float d = 1;
        if(get_real_grid_dimension().x > get_real_grid_dimension().y){ d = (float)dimension.x / (float)get_real_grid_dimension().x;}else{
            d = (float)dimension.y/(float)get_real_grid_dimension().y;
        }
        if(d <= 2.f){ return 2.f; }
        return d;
    }
    Vec2 Grid::get_real_grid_position() {
        //TODO get center of real grid dimension

        //TODO first living cell in row
        Vec2 position_A = Vec2{0,0};
        Vec2 position = Vec2{0,0};
        for(auto c: cells){
            if(c->get_state()) {
                position_A.y = c->get_position().y;
                break;
            }

        }

        // TODO first living cell in column
        bool b = false;
        for(int x=0; x < dimension.x; x++){
            for(int y=0; y < dimension.y; y++){
                if(cells[(y*dimension.x)+x]->get_state()) {
                    position_A.x = cells[(y*dimension.x)+x]->get_position().x;
                    b = true;
                    break;
                }
            }
            if(b){break;}
        }
        position.x = position_A.x;// + (get_real_grid_dimension().x/2);
        position.y = position_A.y;// + (get_real_grid_dimension().y/2);

        return position; // Center of figure
    }
    float Grid::get_scrollx_position() {
        float x = (float)get_real_grid_position().x;//+((float)get_real_grid_dimension().x/2);
        auto x_max = (float)dimension.x;
        float real_x = ((float)get_real_grid_dimension().x+2)*x/x_max;
        return (x+real_x)/x_max;
    }
    float Grid::get_scrolly_position() {

        float y = (float)get_real_grid_position().y;//+((float)get_real_grid_dimension().y/2);
        auto y_max = (float)dimension.y;
        float real_y = ((float)get_real_grid_dimension().y+1)*y/y_max;
        return (y+real_y)/y_max;
    }

    //=========================================================================================== CLASS NEIGHBOURS ===//
    Neighbours::Neighbours(Cell *_owner) {
        owner = _owner;

        owner->set_neighbours(this);        // associate this neighbour to owner Cell

        Vec2 pos_o = owner->get_position(); // position owner
        Grid* grid = owner->get_grid();     // grid

        // add all cells next to this object
        for(int y= -1; y <= 1; y++){
            for(int x= -1; x <= 1; x++){

                // position of next_to cell
                Vec2 pos_n = (Vec2){pos_o.x+x,pos_o.y + y};
                // check x position of next_to cell
                if( (pos_n.x >= 0) && (pos_n.x < owner->get_grid()->get_dimension().x) ){
                    // check y position of next_to cell
                    if( (pos_n.y >= 0) && (pos_n.y < owner->get_grid()->get_dimension().y) ){
                        // filter owner cell object
                        if((pos_o.x != pos_n.x) || (pos_o.y != pos_n.y)){
                            cells.push_back(grid->get_cell(pos_n));
                        }
                    }
                }
            }
        }

    }
    //=============================================================================================== CLASS FIGURE ===//
    Figure::Figure(std::string &path) {
        std::fstream file;

        std::string code;   // whole code as string
        std::string dim;    // dimension as string


        file.open(path);
            // check if file exist else throw std::logic_error
        if(!file.is_open()){ throw std::logic_error("ERROR: could not load file"); }

        while(file){

            std::string row;
            // read line in file and write line into row
            std::getline(file, row);
            // if line starts with a number, 'b', 'o' , '$' or '!' write line into code
            // if line starts with '#' ignore line
            // else write line into dim 'dimension'
            if(((row[0] >= 0x30) && (row[0] <= 0x39)) || (row[0] == 'b') || (row[0] == 'o') || (row[0] == '$') || ((row[0] == '!'))){
                for(auto c: row){
                    code.push_back(c);
                }
            }else if(row[0] != '#'){
                for(auto c : row){
                    dim.push_back(c);
                }
            }
        }

        // get dimension of figure from dim
        dimensions = convert_string_to_dimensions(dim);
        // fill figure_table with converted code
        states = convert_string_to_states(code);


    }
    Figure::Figure(GameOfLife *gameOfLife) {
        // get dimensions from GameOfLife object
        dimensions = gameOfLife->get_grid()->get_dimension();
        // get every cell state in grid
        for(auto c: gameOfLife->get_grid()->get_cells()){
            states.push_back(c->get_state());
        }
    }

    Vec2 Figure::convert_string_to_dimensions(std::string str) {
        std::string x, y;

        int comma = 0;

        for(auto c: str){

            // filter numbers
            if((c >= 0x30) && (c <= 0x39)){
                if(comma == 0){ x.push_back(c); }
                if(comma == 1){ y.push_back(c); }
            }
            // filter comma
            if(c == ','){ comma++; }
            if(comma > 1){ throw std::logic_error("ERROR: there are to much comma's in dimension line"); }
        }

        return (Vec2){std::stoi(x), std::stoi(y)};
    }
    std::string Figure::convert_dimension_to_string() {
        std::string str_dim;

        int x = 0;
        int y = 0;

        // check real row of figure
        int t_y = 0;
        for(int _y = 0; _y < dimensions.y; _y++){
            int t_x = 0;
            for(int _x = 0; _x < dimensions.y; _x++){
                if(states[(_y*dimensions.x)+_x]){t_x = _x;}
            }

            if(x < t_x){ x = t_x; }
            if(t_x > 0){ y = _y; }
            if((t_y == 0) && (_y > t_y) && (t_x > 0)){
                t_y = _y;
            }
        }

        int column_f = 0;
        bool b = false;
        for(int _x=0; _x < dimensions.x; _x++){
            for(int _y=0; _y < dimensions.y; _y++){
                if(states[(_y*dimensions.x+_x)]){ b = true; }
            }
            column_f = _x;
            if(b){break;}

        }
        // convert real dimensions into a string
        str_dim += "x=" + std::to_string(x-column_f+1) + " , y=" + std::to_string(y-t_y+2);


        return str_dim;
    }

    std::vector<bool> Figure::convert_string_to_states(std::string str) {
        std::vector<bool> _states;
        std::string _num;
        int x = 0;
        int y = 0;

        // prepare _states for the right dimensions
        for(int i=0; i < (dimensions.x * dimensions.y); i++){
            _states.push_back(false);
        }

        // check each character in string
        for(auto c: str){
            // if character is a number add to _num buffer
            if((c >=0x30) && (c <= 0x39)){ _num.push_back(c); }
            // if character is a 'b' set state on position [x;y](*_num) to false and increment x
            if(c == 'b'){
                if(_num.empty()){ _num.push_back('1'); }
                for(int b=0; b < std::stoi(_num); b++){
                    _states[(y *dimensions.x)+(x)]=false;
                    x++;
                }
                // reset _num buffer
                _num.clear();
            }
            // if character is a 'o' set state on position [x;y](*_num) to true and increment x
            if(c == 'o'){
                if(_num.empty()){ _num.push_back('1'); }
                for(int o=0; o < std::stoi(_num); o++){
                    _states[(y*dimensions.x)+(x)]=true;
                    x++;
                }
                // reset _num buffer
                _num.clear();
            }
            // if character is a '$' increment y; reset _num buffer; set x to 0
            if(c == '$'){
                if(_num.empty()){ _num.push_back('1');}
                for(int r=0; r < std::stoi(_num); r++){
                    y++;
                }
                _num.clear();
                x=0;
            }
            // EOF "End Of File"
            if(c == '!'){ break; }
        }
        return _states;
    }
    std::string Figure::convert_states_to_string() {
        std::string str_states;

        // convert states to string
        for(int y=0; y < dimensions.y; y++){
            for(int x=0; x < dimensions.x; x++){
                if(states[(y*dimensions.x)+x]){ str_states += 'o'; }else{ str_states += 'b'; }
            }
            str_states += '$';
        }
        // count first empty columns
        int column_f = 0;
        bool b = false;
        for(int _x=0; _x < dimensions.x; _x++){
            for(int _y=0; _y < dimensions.y; _y++){
                if(states[(_y*dimensions.x+_x)]){ b = true; }
            }
            column_f = _x;
            if(b){break;}

        }
        // filter first columns
        b = true;
        int counter = 0;
        std::string _str_states;
        for(auto c: str_states){
            if(c == '$'){ _str_states.push_back(c); b = true;}else
            if(b == true){
                counter++;
                if(counter >= column_f){ b = false; counter = 0;}
            }else
            { _str_states.push_back(c);}

        }

        str_states = _str_states;

        // filter last death columns
        counter = 0;
        char last = 0x00;
        _str_states.clear();
        for(auto c: str_states){
            if(last == 0x00){last = c;}
            if(c == last){ counter++;}
            if(c != last){
                if((c == '$') && (last == 'b')){ counter = 1; last = 0x00; }
                if(counter > 1){ _str_states += std::to_string(counter) + last; counter = 1; } else {_str_states += last;}
                last = c;
            }
        }

        int counter_f = 0; // counter first empty rows
        int counter_l = 0; // counter last empty rows
        b = false;    // first row marker
        for(auto c: _str_states){
            if((c=='$')&&(b == false)){ counter_f+=2; } //count first empty rows
            if((c=='$')&&(b == true)){ counter_l+=2; }  //count last empty rows
            if((c == 'o') || (c == 'b')){counter_l = 0; b = 1; }
        }
        // delete first empty rows
        _str_states.erase(0,counter_f);

        // delete last empty rows
        while(counter_l > 0){
            _str_states.pop_back();
            counter_l--;
        }
        _str_states += '!';

        str_states.clear();
        for(auto c: _str_states){
            if(c != 0x00){str_states.push_back(c);}
        }
        return str_states;
    }

    int Figure::save_to_file(std::string &path) {
        std::fstream file;

        // create file and open
        file.open(path , std::ios_base::out);
        // Error Handling
        if(!file.is_open()){ throw std::logic_error("ERROR: failed"); }

        file << convert_dimension_to_string() << "\n";
        file << convert_states_to_string();

        // close file
        file.close();

        return 0;
    }
    //========================================================================================= CLASS GAME OF LIFE ===//
    GameOfLife::GameOfLife(Vec2 _dimensions, Rule _rules, RuleColor _rulesColor) {
        rules = _rules;                                         // initialize rules
        grid = new Grid(this, _dimensions); // initialize new grid
        rules_color = _rulesColor;                              // initialize color Rules
    }

    bool GameOfLife::check_rules_in_cell(Cell *_cell) {
        int counter = 0;
        // count every true cell in neighbours
        for(auto n: _cell->get_neighbours()->get_cells()){
            if(n->get_state()){ counter++; }
        }
        // check rules
        if (rules.alive[counter]){ return true; }
        if (rules.death[counter]){ return false; }



        // returns new cell state
        return _cell->get_state();
    }
    void GameOfLife::check_rules_in_grid() {
        std::vector<bool> _states;  // buffer for cell states

        // check rules for every cell in grid and push result into state buffer
        for(auto c: grid->get_cells()){
            _states.push_back(check_rules_in_cell(c));
        }
        // write new state from buffer to cells in grid
        for(auto c: grid->get_cells()){
            c->set_state(_states[(c->get_position().y*grid->get_dimension().x)+c->get_position().x]);
        }

        // change color
        for(auto c: grid->get_cells()){
            c->get_neighbours()->set_n_alive(0);
            for(auto n: c->get_neighbours()->get_cells()){
                if(n->get_state()){ c->get_neighbours()->set_n_alive(c->get_neighbours()->get_n_alive()+1); }
            }
            c->set_color(rules_color.color[c->get_neighbours()->get_n_alive()]);
        }

    }

    void GameOfLife::clear_grid() {
        // set every cell state in grit to false
        for(auto c: grid->get_cells()){
            c->set_state(false);
        }
    }
    void GameOfLife::populate_figure(Figure *_figure, Vec2 position, int angle) {
        int x_c = 0;

        // Error Handler
        if((angle < 0)||(angle > 3)){throw std::logic_error("ERROR wrong angle in rotate_and_populate_figure()"); }
        // no roation
        if(angle == 0){
            // Error handling
            if(_figure->get_dimension().x+position.x > grid->get_dimension().x){throw std::logic_error("ERROR: figure dimension x is larger than grid dimension x");}
            if(_figure->get_dimension().y+position.y > grid->get_dimension().y){throw std::logic_error("ERROR: figure dimension y is larger than grid dimension y");}
            // insert figure states into grid
            for(int _y=0; _y < _figure->get_dimension().y; _y++){
                for(int _x=0; _x < _figure->get_dimension().x; _x++){
                    grid->get_cell((Vec2){_x + position.x, _y + position.y})->set_state(_figure->get_state((Vec2){_x, _y}));
                }
            }
        }

        // rotate 180°
        if(angle == 2){
            // Error handling
            if(_figure->get_dimension().x > grid->get_dimension().x){throw std::logic_error("ERROR: figure dimension x is larger than grid dimension x");}
            if(_figure->get_dimension().y > grid->get_dimension().y){throw std::logic_error("ERROR: figure dimension y is larger than grid dimension y");}
            // insert figure states into grid
            for(int _y=_figure->get_dimension().y-1; _y >= 0; _y--){
                int y_c = 0;
                for(int _x=_figure->get_dimension().x-1; _x >= 0; _x--){
                    grid->get_cell((Vec2){y_c + position.x, x_c + position.y})->set_state(_figure->get_state((Vec2){_x, _y}));
                    y_c++;
                }
                x_c++;
            }
        }

        // rotate 90°
        if(angle == 1){
            // Error Handling
            if(_figure->get_dimension().x > grid->get_dimension().y){throw std::logic_error("ERROR: figure dimension x is larger than grid dimension y");}
            if(_figure->get_dimension().y > grid->get_dimension().x){throw std::logic_error("ERROR: figure dimension y is larger than grid dimension x");}
            // insert figure states into grid
            for(int _x=0; _x < _figure->get_dimension().x; _x++){
                int y_c = 0;
                for(int _y=_figure->get_dimension().y-1; _y >= 0; _y--){
                    grid->get_cell((Vec2){y_c + position.x, x_c + position.y})->set_state(_figure->get_state((Vec2){_x,_y}));
                    y_c++;
                }
                x_c++;
            }
        }

        // rotate 270°
        if(angle == 3){
            // Error Handling
            if(_figure->get_dimension().x > grid->get_dimension().y){throw std::logic_error("ERROR: figure dimension x is larger than grid dimension y");}
            if(_figure->get_dimension().y > grid->get_dimension().x){throw std::logic_error("ERROR: figure dimension y is larger than grid dimension x");}
            // insert figure states into grid
            for(int _x=_figure->get_dimension().x-1; _x >= 0; _x--){
                int y_c = 0;
                for(int _y=0; _y < _figure->get_dimension().y; _y++){
                    grid->get_cell((Vec2){y_c + position.x, x_c + position.y})->set_state(_figure->get_state((Vec2){_x, _y}));
                    y_c++;
                }
                x_c++;
            }
        }
    }

    bool GameOfLife::random_bool() {
            if (rand() % 2 == 0){return true;}
            return false;
    }
    void GameOfLife::populate_random() {
        for(int _y=0; _y < grid->get_dimension().y; _y++){
            for(int _x=0; _x < grid->get_dimension().x; _x++){
                grid->get_cell((Vec2){_x, _y})->set_state(random_bool());
            }
        }
        // Colorize Grid
        for(auto c: grid->get_cells()){
            c->get_neighbours()->set_n_alive(0);
            for(auto n: c->get_neighbours()->get_cells()){
                if(n->get_state()){ c->get_neighbours()->set_n_alive(c->get_neighbours()->get_n_alive()+1); }
            }
            c->set_color(rules_color.color[c->get_neighbours()->get_n_alive()]);
        }
    }
    int GameOfLife::runUI() {


        // Setup SDL // https://github.com/ocornut/imgui.git
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
        {
            printf("Error: %s\n", SDL_GetError());
            return -1;
        }


        // From 2.0.18: Enable native IME. // https://github.com/ocornut/imgui.git
        #ifdef SDL_HINT_IME_SHOW_UI
        SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
        #endif


        // Setup window // https://github.com/ocornut/imgui.git
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        SDL_Window* window = SDL_CreateWindow("Cornway's Game Of Life", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
        SDL_GLContext gl_context = SDL_GL_CreateContext(window);

        SDL_GL_MakeCurrent(window, gl_context);
        SDL_GL_SetSwapInterval(1); // Enable vsync

        // Setup Dear ImGui context // https://github.com/ocornut/imgui.git
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
        //io.ConfigViewportsNoAutoMerge = true;
        //io.ConfigViewportsNoTaskBarIcon = true;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones. // https://github.com/ocornut/imgui.git
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            style.ChildBorderSize = 0.0f;
        }

        // Setup Platform/Renderer backends // https://github.com/ocornut/imgui.git
        ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
        ImGui_ImplOpenGL2_Init();

        ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
        // Main loop
        bool done = false;
        // -------------------------------------------------------------------------------
        // USER CODE GLOBAL BEGIN --------------------------------------------------------
        UI_Flag uiFlag = UI_Flag_none;
        uiFlag |= UI_Flag_enable_GridUI | UI_Flag_enable_SetupUI;

        UI_Options_Flag uiOptFlag = UI_Options_Flag_none;

        std::string message;                                    // message for Message User Interface

        float zoom = 2.f;                                           // zoom variable
        int speed = 0, count=0;                                 // speed variables

        Figure* reset_figure = new Figure(this);     // Reset figure object
        Figure_Stack stack = {};                               // Stack needed for step back -> max size defined ba STACK_SIZE

        std::string p;                                         // filepath as string

        // USER CODE GLOBAL END ----------------------------------------------------------
        // -------------------------------------------------------------------------------
        // main loop begin
        while (!done) {

            // Poll and handle events (inputs, window resize, etc.) // https://github.com/ocornut/imgui.git
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT)
                    done = true;
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                    done = true;
            }

            ImGui_ImplOpenGL2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
            //----------------------------------------------------------------------------------------------------------
            // USER CODE FRAME BEGIN -----------------------------------------------------------------------------------
            // initialize Grid User Interface if Grid_UI is true
            UI::Dockspace_UI();

            if(uiFlag & UI_Flag_enable_GridUI) {

                UI::grid_UI(/*io,*/ uiFlag, uiOptFlag, *this, stack, zoom, speed, count);
            }
            // initialize Setting User Interface if Setting_UI is true
            if(uiFlag & UI_Flag_enable_SetupUI) {
                UI::setting_UI(uiFlag, uiOptFlag, *this, *reset_figure, stack, zoom, speed);
            }
            // initialize Load User Interface if Load_UI is true
            if(uiFlag & UI_Flag_enable_LoadUI){
                UI::load_UI(uiFlag, uiOptFlag, *this, *reset_figure, message, zoom); // TODO remove zoom input variable
            }
            // initialize Message User Interface if Message_UI is true
            if(uiFlag & UI_Flag_enable_MessageUI){
                UI::messageBox_UI(uiFlag, message);
            }
            // initialize Screenshot User Interface if Save_UI is true
            if(uiFlag & UI_Flag_enable_SaveUI){
                UI::screenshot_UI(uiFlag ,*this, message );
            }
            if(uiFlag & UI_Flag_enable_LoadExampleUI) {
                UI::load_example_UI(uiFlag, uiOptFlag, *this, *reset_figure, zoom, speed);
            }
            // USER CODE FRAME END -------------------------------------------------------------------------------------
            //----------------------------------------------------------------------------------------------------------


            // Rendering // https://github.com/ocornut/imgui.git
            ImGui::Render();
            glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

            // Update and Render additional Platform Windows // https://github.com/ocornut/imgui.git
            // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
            //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
                SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
            }

            SDL_GL_SwapWindow(window);
        }



        // Cleanup // https://github.com/ocornut/imgui.git
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        //SDL_GLContext gl_context = SDL_GL_CreateContext(window);
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit(); //

        return 0;

    }

    //=================================================================================================== CLASS UI ===//
    void UI::print(GameOfLife *_gameOfLife, float zoom) {

        ImVec2 p = ImGui::GetCursorScreenPos();

        // Create new child for scrolling
        ImVec2 scrolling_child_size = ImVec2(ImGui::GetFrameHeight() + (float)_gameOfLife->get_grid()->get_dimension().x*zoom, (float)_gameOfLife->get_grid()->get_dimension().y*zoom);
        ImGui::BeginChild("scrolling", scrolling_child_size, true,  ImGuiWindowFlags_HorizontalScrollbar);
        // create for every living cell a white rectangle
        for(int y = 0; y < _gameOfLife->get_grid()->get_dimension().y; y++){
            for(int x = 0; x < _gameOfLife->get_grid()->get_dimension().x; x++){
                if(_gameOfLife->get_grid()->get_cell((Vec2){x,y})->get_state()){
                    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x+((float)x*zoom)+1, p.y+((float)y*zoom)+1), ImVec2(p.x+((float)x*zoom)+zoom, p.y+((float)y*zoom)+zoom), _gameOfLife->get_grid()->get_cell((Vec2){x,y})->get_color());
                    //ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x+(x*zoom), p.y+(y*zoom)), ImVec2(p.x+(x*zoom)+zoom, p.y+(y*zoom)+zoom), _gameOfLife->get_grid()->get_cell((Vec2){x,y})->get_color());

                }
            }
        }


        ImGui::EndChild();
    }

    void UI::button_setting_zoom(UI_Options_Flag &uiOptionsFlag, float &factor) {

        static bool auto_zoom;
        if(uiOptionsFlag & UI_Options_Flag_enable_AutoZoom){ auto_zoom = true; }else{ auto_zoom = false; }
        ImGui::Checkbox("Auto Zoom", &auto_zoom);
        if(auto_zoom){uiOptionsFlag |= UI_Options_Flag_enable_AutoZoom; }else{uiOptionsFlag &= ~UI_Options_Flag_enable_AutoZoom; }

        // Zoom in button
        const ImVec2 size = ImVec2(96,20.);
        if(!(uiOptionsFlag & UI_Options_Flag_enable_AutoZoom)) {
            if (ImGui::Button("Zoom +", size)) {

                factor +=2;
            }
            ImGui::SameLine();
            // Zoom out button
            if (ImGui::Button("Zoom -", size)) {
                if (factor > 2) { factor -=2 ; }else{ factor = 2; }
            }
        }
    }
    void UI::button_setting_speed(int &factor) {
        // speed up button
        const ImVec2 size = ImVec2(96,20.);
        if (ImGui::Button("Speed +",size)) {
            if(factor > 0){factor--;}
        }
        ImGui::SameLine();
        // speed down button
        if (ImGui::Button("speed -",size)) {
            factor++;
        }
    }
    void UI::button_setting_run(UI_Options_Flag &uiOptionsFlag) {
        const ImVec2 size_run = ImVec2(96,20.);
        if(!(uiOptionsFlag & UI_Options_Flag_Run)) {
            ImGui::PushID(2);
            //color green
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4) ImColor::HSV(2 / 7.0f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4) ImColor::HSV(2 / 7.0f, 0.8f, 0.8f));
            // create button
            if (ImGui::Button("RUN", size_run)) {
                uiOptionsFlag |= UI_Options_Flag_Run;
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }else{
            ImGui::PushID(0);
            //color red
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.8f));
            // create button
            if (ImGui::Button("STOP", size_run)) {
                uiOptionsFlag &= ~UI_Options_Flag_Run;
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }
    }
    /*void UI::button_setting_stop(UI_Options_Flag &uiOptionsFlag) {
        const ImVec2 size_stop = ImVec2(44,20.);
        ImGui::PushID(0);
        //color red
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.8f));
        // create button
        if (ImGui::Button("STOP", size_stop)) {
            uiOptionsFlag &= ~UI_Options_Flag_Run;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }*/
    void UI::button_setting_reset(GameOfLife &gameOfLife, Figure &reset_figure, Figure_Stack &figureStack) {
        const ImVec2 size_res = ImVec2(44,20.);
        // create button
        if (ImGui::Button("reset", size_res)) {
            gameOfLife.clear_grid();
            gameOfLife.populate_figure(&reset_figure, (Vec2){0,0},0);
            //Colorize grid
            for(auto c: gameOfLife.get_grid()->get_cells()){
                c->get_neighbours()->set_n_alive(0);
                for(auto n: c->get_neighbours()->get_cells()){
                    if(n->get_state()){ c->get_neighbours()->set_n_alive(c->get_neighbours()->get_n_alive()+1); }
                }
                c->set_color(gameOfLife.get_color_rules()->color[c->get_neighbours()->get_n_alive()]);
            }

            figureStack.clear();
        }
    }
    void UI::button_setting_step(GameOfLife &gameOfLife, Figure_Stack &_stack) {
        const ImVec2 size = ImVec2(96,20.);
        // create back button
        if (ImGui::Button("<< Step", size)) {
            if(!_stack.empty()){

                gameOfLife.populate_figure(_stack.back(), Vec2{0, 0}, 0);
                _stack.pop_back();
                // COLORIZE
                for(auto c: gameOfLife.get_grid()->get_cells()){
                    c->get_neighbours()->set_n_alive(0);
                    for(auto n: c->get_neighbours()->get_cells()){
                        if(n->get_state()){ c->get_neighbours()->set_n_alive(c->get_neighbours()->get_n_alive()+1);}
                    }
                    c->set_color(gameOfLife.get_color_rules()->color[c->get_neighbours()->get_n_alive()]);
                }
            }
        }
        ImGui::SameLine();
        // step for button
        if (ImGui::Button("Step >>", size)) {
            _stack.push_back(new Figure(&gameOfLife));
            if(_stack.size() > STACK_SIZE){ _stack.pop_front(); }
            gameOfLife.refresh_grid();

        }
    }
    void UI::button_setting_clear(GameOfLife &gameOfLife, Figure &reset_figure, Figure_Stack &figureStack) {
        const ImVec2 size_clr = ImVec2(44,20.);
        if (ImGui::Button("clear",size_clr)) {
            gameOfLife.clear_grid();
            reset_figure = Figure(&gameOfLife);
            figureStack.clear();
        }
    }
    void UI::button_setting_random(GameOfLife &gameOfLife, Figure &reset_figure,Figure_Stack &figureStack) {
        const ImVec2 size_rand = ImVec2(200.,20.);
        if (ImGui::Button("random", size_rand)) {
            gameOfLife.clear_grid();
            gameOfLife.populate_random();
            reset_figure = Figure(&gameOfLife);
            figureStack.clear();
        }
    }
    void UI::button_setting_load(UI_Flag &uiFlag) {
        const ImVec2 size_load = ImVec2(200.,20.);
        if (ImGui::Button("load from file", size_load)) {
            uiFlag |= UI_Flag_enable_LoadUI;
        }
    }
    void UI::button_setting_load_exp(UI_Flag &uiFlag) {
        const ImVec2 size_load = ImVec2(200.,20.);
        if (ImGui::Button("load example", size_load)) {
            uiFlag |= UI_Flag_enable_LoadExampleUI;
        }
    }
    void UI::button_setting_screenshot(UI_Flag &uiFlag) {
        const ImVec2 size = ImVec2(200.,20.);
        if (ImGui::Button("screenshot to .lif file", size)) {
            uiFlag |= UI_Flag_enable_SaveUI;
        }
    }
    void UI::button_load(UI_Flag &uiFlag, UI_Options_Flag &uiOptionsFlag, GameOfLife &gameOfLife, Figure &reset_figure, std::string path, Vec2 position, int &angle, std::string &message, float &zoom) {
        ImGui::PushID(2);
        //color green
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2 / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.8f));
        if (ImGui::Button("Load")) {

            try {
                std::string _path = (std::string) std::move(path);
                auto *_f = new gol::Figure(_path);

                // ERROR HANDLING
                if(_f->get_dimension().x+position.x > gameOfLife.get_grid()->get_dimension().x){throw std::logic_error("ERROR: Figure Dimension x in comination with position x are to large");}
                if(_f->get_dimension().y+position.y > gameOfLife.get_grid()->get_dimension().y){throw std::logic_error("ERROR: Figure Dimension y in comination with position y are to large");}


                gameOfLife.populate_figure(_f, position, (angle / 90));
                reset_figure = Figure(&gameOfLife);
                uiFlag &= ~UI_Flag_enable_LoadUI;
                delete _f;

                //Colorize grid
                for(auto c: gameOfLife.get_grid()->get_cells()){
                    c->get_neighbours()->set_n_alive(0);
                    for(auto n: c->get_neighbours()->get_cells()){
                        if(n->get_state()){ c->get_neighbours()->set_n_alive(c->get_neighbours()->get_n_alive()+1); }
                    }
                    c->set_color(gameOfLife.get_color_rules()->color[c->get_neighbours()->get_n_alive()]);
                }
                //zoom = gameOfLife.get_grid()->get_auto_zoom_factor();
                uiOptionsFlag |= UI_Options_Flag_enable_AutoZoom | UI_Options_Flag_enable_AutoScroll;
            }catch(std::logic_error &err){
                message = err.what();
                uiFlag |= UI_Flag_enable_MessageUI;
            }



        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }
    void UI::button_save(UI_Flag &uiFlag, GameOfLife &gameOfLife, std::string path) {
        //color green
        ImGui::PushID(2);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2 / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.8f));
        if (ImGui::Button("Save")) {
            std::string Filename(path);

            //Grid* grid = gameOfLife.get_grid();

            auto *_fig = new gol::Figure(&gameOfLife);

            _fig->save_to_file(path);

            uiFlag &= ~UI_Flag_enable_SaveUI;

        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }

    void UI::setting_UI(UI_Flag &uiFlag, UI_Options_Flag &uiOptionsFlag, GameOfLife &gameOfLife, Figure &reset_figure, Figure_Stack &figureStack,float &zoom, int &speed) {

        ImGui::Begin("Settings", nullptr, /*ImGuiWindowFlags_NoMove |*/ ImGuiWindowFlags_NoDecoration);

        //Autoscroll Checkbox Begin
        bool autoScroll;
        if(uiOptionsFlag & UI_Options_Flag_enable_AutoScroll){ autoScroll = true; }else{ autoScroll = false; }
        ImGui::Checkbox("Auto Scroll", &autoScroll);
        if(autoScroll){ uiOptionsFlag |= UI_Options_Flag_enable_AutoScroll;}else{uiOptionsFlag &= ~UI_Options_Flag_enable_AutoScroll;}
        //Autoscroll Checkbox End

        UI::button_setting_zoom(uiOptionsFlag, zoom);      // create zoom button

        UI::button_setting_speed(speed);    // create speed button
        UI::button_setting_step(gameOfLife, figureStack); // create step button
        ImGui::Spacing();
        UI::button_setting_run(uiOptionsFlag);        // run game of life button
        //ImGui::SameLine();
        //UI::button_setting_stop(uiOptionsFlag);       // stop game of life button

        ImGui::SameLine();
        UI::button_setting_reset(gameOfLife, reset_figure, figureStack); // reset game of life button

        ImGui::SameLine();
        UI::button_setting_clear(gameOfLife, reset_figure, figureStack); // clear game of life button

        ImGui::Spacing();
        UI::button_setting_random(gameOfLife, reset_figure, figureStack); // random button

        UI::button_setting_load(uiFlag);   // load from file button
        UI::button_setting_load_exp(uiFlag);
        UI::button_setting_screenshot(uiFlag);
        ImGui::Spacing();

        UI::setup_rules(uiOptionsFlag, gameOfLife);    // rules setting checkboxes

        ImGui::End();

    }

    void UI::Dockspace_UI(){
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        window_flags |= ImGuiWindowFlags_NoBackground ;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGui::Begin("DockSpace", nullptr, window_flags);

        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        ImGui::End();
    }
    void UI::grid_UI(UI_Flag &uiFlag, UI_Options_Flag &uiOptionsFlag, GameOfLife &gameOfLife, Figure_Stack &figureStack, float &zoom, int &speed, int &count) {




        ImGui::Begin("GRID",NULL,ImGuiWindowFlags_HorizontalScrollbar /*| ImGuiWindowFlags_NoMove */| ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);

        //Auto Scroll Begin
        if(uiOptionsFlag & UI_Options_Flag_enable_AutoScroll) {

            ImGui::SetScrollX(gameOfLife.get_grid()->get_scrollx_position()*(ImGui::GetScrollMaxX()));  //TODO
            ImGui::SetScrollY(gameOfLife.get_grid()->get_scrolly_position()*(ImGui::GetScrollMaxY())); //TODO

            //std::cerr << ImGui::GetContentRegionMax().x << "x" << ImGui::GetContentRegionMax().y << " <- ";
            //std::cerr << ImGui::GetScrollMaxX() << "x" << ImGui::GetScrollMaxY() << "\n"; //TODO debug
        }
        //Auto Scroll End
        //Auto Zoom Begin
        if(uiOptionsFlag & UI_Options_Flag_enable_AutoZoom) {
            if ((gameOfLife.get_grid()->get_real_grid_dimension().x > 0) && (gameOfLife.get_grid()->get_real_grid_dimension().y > 0)) {
                zoom = gameOfLife.get_grid()->get_auto_zoom_factor();
            }
        }
        //Auto Zoom End



        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::Text("Application average %.3f ms/frame (%.2f FPS)", 1000.0f / io.Framerate, io.Framerate); //TODO: FOR DEBUG

        // Menubar
        static bool setting = false;
        static bool run;
        if(uiFlag & UI_Flag_enable_SetupUI){ setting = true; }
        if(uiOptionsFlag & UI_Options_Flag_Run){ run = true; }else{run = false; }
        if (ImGui::BeginMenuBar()){
            if (ImGui::BeginMenu("Options")){
                ImGui::MenuItem("Settings", NULL, &setting);
                ImGui::MenuItem("run", NULL, &run); //TODO
                ImGui::EndMenu();
            }

        }
        ImGui::EndMenuBar();
        if(setting){ uiFlag |= UI_Flag_enable_SetupUI; }else{ uiFlag &= ~UI_Flag_enable_SetupUI; }
        if(run){ uiOptionsFlag |= UI_Options_Flag_Run; }else{ uiOptionsFlag &= ~UI_Options_Flag_Run; }
        // print grid to imGUI figure
        UI::print(&gameOfLife, zoom);

        // run
        if (uiOptionsFlag & UI_Options_Flag_Run) {
            if (count > speed) { count = speed; }
            if (count == speed) {
                figureStack.push_back(new Figure(&gameOfLife));
                if(figureStack.size() > STACK_SIZE){ figureStack.pop_front(); }
                gameOfLife.refresh_grid();
                count = 0;
            } else { count++; }
        }

        ImGui::End();
    }
    void UI::load_UI(UI_Flag &uiFlag, UI_Options_Flag &uiOptionsFlag, GameOfLife &gameOfLife, Figure &reset_figure, std::string &message, float &zoom) {
        static char path[256] = "/Users/user404/Studium/CE27_Softwaretechnik/GIT/gameoflife/SourceCode/example_pattern/toggle.lif";
        static int x_pos = 0;
        static int y_pos = 0;
        static int angle = 0;

        ImGui::Begin("Load from File", nullptr, 0/*ImGuiWindowFlags_NoMove*/  );
        ImGui::Text("enter full filepath");
        ImGui::InputText("filepath", path, IM_ARRAYSIZE(path)); //Text input path


        ImGui::Text("enter x and y position of figure in grid");
        ImGui::InputInt("x", &x_pos); // Textfield x position
        if(x_pos > gameOfLife.get_grid()->get_dimension().x){x_pos = gameOfLife.get_grid()->get_dimension().x; }
        if(x_pos < 0){x_pos = 0;}

        ImGui::InputInt("y", &y_pos); // Textfield y position
        if(y_pos > gameOfLife.get_grid()->get_dimension().y){ y_pos = gameOfLife.get_grid()->get_dimension().y; }
        if(y_pos < 0){y_pos = 0;}


        ImGui::Text("enter angle of figure in grid");
        ImGui::InputInt("angle", &angle, 90, 90, 0); // Textfield x position
        if(angle > 270) {
            angle = 270;
        }
        if(angle < 0){
            angle = 0;
        }

        // Button load from file
        try {
            button_load(uiFlag, uiOptionsFlag, gameOfLife, reset_figure, (std::string) path, (Vec2) {x_pos, y_pos}, angle, message, zoom);
        }catch(std::logic_error &err){
            message = (std::string)err.what();
            uiFlag |= UI_Flag_enable_MessageUI;
        }
        // abort button
        ImGui::SameLine();
        //color red
        ImGui::PushID(0);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.8f));
        if (ImGui::Button("Abort")) {
            uiFlag &= ~UI_Flag_enable_LoadUI;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        ImGui::End();

        //TODO add some standard figures
    }
    void UI::screenshot_UI(UI_Flag &uiFlag, GameOfLife &gameOfLife, std::string &message){
        static char path[256] = "/Users/user404/Studium/CE27_Softwaretechnik/PROTO_GOL/test_Screenshot.lif";

        ImGui::Begin("Save to File", nullptr, 0/*ImGuiWindowFlags_NoMove*/);
        ImGui::Text("enter full filepath");
        ImGui::InputText("filepath", path, IM_ARRAYSIZE(path)); //Text input path

        try {
            button_save(uiFlag,gameOfLife, path);
        }catch(std::logic_error &err){
            message = (std::string)err.what();
            uiFlag |= UI_Flag_enable_MessageUI;
        }
        // abort button
        ImGui::SameLine();
        ImGui::PushID(0);
        //color red
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.8f));
        if (ImGui::Button("Abort")) {
            uiFlag &= ~UI_Flag_enable_SaveUI;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();


        ImGui::End();
    }
    void UI::messageBox_UI(UI_Flag &uiFlag, std::string &message) {
        ImGui::Begin("Message");

        ImGui::Text("%s", message.c_str());

        if (ImGui::Button("OK")) {
            uiFlag &= ~UI_Flag_enable_MessageUI;
            message.clear();
        }

        ImGui::End();
    }
    void UI::ColorPicker(GameOfLife &gameOfLife, const char* label, ImU32 *color){
        //ImGui::Begin("Color Picker");
            float col[4];
            col[0] = (float)((*color      ) & 0xFF) / 255.0f;
            col[1] = (float)((*color >> 8 ) & 0xFF) / 255.0f;
            col[2] = (float)((*color >> 16) & 0xFF) / 255.0f;
            col[3] = (float)((*color >> 24) & 0xFF) / 255.0f;

            static bool drag_and_drop = false;
            static bool hdr = true;
            static bool alpha_preview = true;
            static bool alpha_half_preview = true;
            static bool options_menu = true;
            ImGuiColorEditFlags misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | (drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);
            //ImGui::ColorPicker4("##picker", &col[0], misc_flags | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
            //ImGui::ColorButton("##current", (ImVec4){col[0],col[1],col[2],col[3]}, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
            //ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
            //ImGui::ColorButton("##palette", (ImVec4){col[0],col[1],col[2],col[3]}, palette_button_flags, ImVec2(20, 20));

            ImGui::ColorEdit4(label, &col[0], ImGuiColorEditFlags_NoInputs | misc_flags);


            ImU32 _col = ((ImU32)(col[0] * 255.0f)      ) |
                         ((ImU32)(col[1] * 255.0f) <<  8) |
                         ((ImU32)(col[2] * 255.0f) << 16) |
                         ((ImU32)(col[3] * 255.0f) << 24);

            *color = _col;




        //ImGui::End();
    }
    void UI::setup_rules(UI_Options_Flag &uiOptionsFlag, GameOfLife &gameOfLife) {
        ImGui::Text("Rules"); //ImGui::SameLine();
        for(int i=0; i < 9; i++) {
            std::string label_a = std::to_string(i);
            label_a += " Alive";
            std::string label_d = std::to_string(i);
            label_d += " Dead";

            bool d = gameOfLife.get_rules()->death[i];
            bool a = gameOfLife.get_rules()->alive[i];

            ImGui::Checkbox(label_a.c_str(), &gameOfLife.get_rules()->alive[i]);
            ImGui::SameLine();
            ImGui::Checkbox(label_d.c_str(), &gameOfLife.get_rules()->death[i]);
            if (((gameOfLife.get_rules()->alive[i])) && (gameOfLife.get_rules()->death[i])) {
                if (d && !a) {
                    gameOfLife.get_rules()->death[i] = false;
                    gameOfLife.get_rules()->alive[i] = true;
                } else {
                    gameOfLife.get_rules()->death[i] = true;
                    gameOfLife.get_rules()->alive[i] = false;
                }

            }


            // Color Picker
            if (!(uiOptionsFlag & UI_Options_Flag_enable_ColorizeAll)) {
                ImGui::SameLine();
                char index[2] = {char(i + 0x30), 0x00};
                ColorPicker(gameOfLife, index, &gameOfLife.get_color_rules()->color[i]);
            }
        }
        button_setting_reset_colors(gameOfLife.get_color_rules());
        static bool color_all;
        if(uiOptionsFlag & UI_Options_Flag_enable_ColorizeAll){color_all = true; }else{color_all = false;}
        ImGui::Checkbox("color all: ", &color_all);
        if(color_all){uiOptionsFlag |= UI_Options_Flag_enable_ColorizeAll;}else{uiOptionsFlag &= ~UI_Options_Flag_enable_ColorizeAll; }
        if(uiOptionsFlag & UI_Options_Flag_enable_ColorizeAll) {
            ImGui::SameLine();
            ImU32 &color_a = gameOfLife.get_color_rules()->color[0];

            ColorPicker(gameOfLife, " ", &color_a);
            for(unsigned int & i : gameOfLife.get_color_rules()->color){

                i = color_a;
            }

        }
        button_setting_reset_rules(gameOfLife.get_rules());
    }

    void UI::button_setting_reset_rules(Rule *_rule) {
        const ImVec2 size = ImVec2(200.,20.);
        if (ImGui::Button("Cornways", size)) {
            for(int r=0; r < 9; r++){
                // reset game of life rules to cornways (23/3)
                if(r == 3){_rule->alive[r] = true; }else{ _rule->alive[r] = false; }
                if((r == 2) || (r == 3)){ _rule->death[r] = false; }else{ _rule->death[r] = true; }
            }
        }
        if (ImGui::Button("Anti Cornways", size)) {
            for(int r=0; r < 9; r++){
                // reset game of life rules to cornways (56/5)
                if(r == 5){_rule->death[r] = true; }else{_rule->death[r] = false; }
                if((r == 5) || (r == 6)){_rule->alive[r] = false; }else{ _rule->alive[r] = true; }

            }
        }
        if (ImGui::Button("Kopiersystem", size)) {
            for(int r=0; r < 9; r++){
                // reset game of life rules to (1357/1357)
                if((r==1)||(r==3)||(r==5)||(r==7)){_rule->alive[r] = true; }else{_rule->alive[r] = false; }
                if((r==1)||(r==3)||(r==5)||(r==7)){_rule->death[r] = false; }else{_rule->death[r] = true; }
            }
        }
        if (ImGui::Button("Anti Kopiersystem", size)) {
            for(int r=0; r < 9; r++){
                // reset game of life rules to (1357/1357)
                if((r==1)||(r==3)||(r==5)||(r==7)){_rule->death[r] = true; }else{_rule->death[r] = false; }
                if((r==1)||(r==3)||(r==5)||(r==7)){_rule->alive[r] = false; }else{_rule->alive[r] = true; }
            }
        }


    }
    void UI::button_setting_reset_colors(RuleColor* _rule){

        const ImVec2 size = ImVec2(200.,20.);
        if (ImGui::Button("Reset colors", size)) {
            // all colors are white
            *_rule = gol::RuleColor{
                    IM_COL32(255,0,0,255),
                    IM_COL32(198,57,0,255),
                    IM_COL32(141,114,0,255),
                    IM_COL32(84,171,0,255),
                    IM_COL32(0,255,0,255),
                    IM_COL32(0,198,57,255),
                    IM_COL32(0,141,114,255),
                    IM_COL32(0,84,171,255),
                    IM_COL32(0,0,255,255)
            };
        }

    }

    void UI::load_example_UI(UI_Flag &uiFlag, UI_Options_Flag &uiOptionsFlag, GameOfLife &gameOfLife, Figure &reset_figure, float &zoom, int &speed) {
        static int x_pos = 0;
        static int y_pos = 0;
        static int angle = 0;

        static bool example_flags[] = {true,false,false,false,false,false,false,false};
        bool last_flags[8];




        for(int i=0; i < 8; i++){
            last_flags[i] = example_flags[i];
        }

        ImGui::Begin("Load example figures", nullptr, 0/*ImGuiWindowFlags_NoMove*/);

        ImGui::Text("Cornway's example figures");
        ImGui::Checkbox("Glider", &example_flags[0]);
        ImGui::Checkbox("Light-Weight Spaceship", &example_flags[1]);
        ImGui::Checkbox("Middle-Weight Spaceship", &example_flags[2]);
        ImGui::Checkbox("Heavy-Weight Spaceship", &example_flags[3]);
        ImGui::Checkbox("Gosper Gun", &example_flags[4]);
        ImGui::Checkbox("Eater", &example_flags[5]);

        ImGui::Text("\nother example figures");
        ImGui::Checkbox("HTW Logo", &example_flags[6]);
        ImGui::Checkbox("Pacman", &example_flags[7]);

        bool equal = true;
        for(int i=0; i < 8; i++){
            if(example_flags[i] != last_flags[i]){
                equal = false;
            }
        }

        if(!equal){
            for(int i=0; i < 8; i++){
                example_flags[i] =  (example_flags[i] + last_flags[i])%2; // XOR
            }
        }

        ImGui::Text("enter x and y position ");
        ImGui::InputInt("x", &x_pos); // Textfield x position
        if(x_pos > gameOfLife.get_grid()->get_dimension().x){x_pos = gameOfLife.get_grid()->get_dimension().x; }
        if(x_pos < 0){x_pos = 0;}

        ImGui::InputInt("y", &y_pos); // Textfield y position
        if(y_pos > gameOfLife.get_grid()->get_dimension().y){ y_pos = gameOfLife.get_grid()->get_dimension().y; }
        if(y_pos < 0){y_pos = 0;}


        ImGui::Text("enter angle of figure");
        ImGui::InputInt("angle", &angle, 90, 90, 0); // Textfield x position
        if(angle > 270) {
            angle = 270;
        }
        if(angle < 0){
            angle = 0;
        }

        ImGui::PushID(2);
        //color green
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2 / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.8f));
        if (ImGui::Button("Load")) {
            gol::Rule r_cornway = (gol::Rule){{false,false, false, true, false, false, false, false, false},
                                              {true, true, false, false, true, true, true, true, true}};

            gol::Rule r_copy = (gol::Rule){{false,true, false, true, false, true, false, true, false},
                                           {true, false, true, false, true, false, true, false, true}};

            if(example_flags[0]){
                //Figure *f = new Figure(glider, glider_dim);

                uiOptionsFlag |= UI_Options_Flag_enable_AutoScroll | UI_Options_Flag_enable_AutoZoom;
                std::string glider_string = "bob$2bo$3o!";
                Vec2 glider_dim = {3, 3};
                auto *f = new Figure(glider_string, glider_dim);
                gameOfLife.populate_figure(f,(Vec2){x_pos,y_pos}, angle/90);
                gameOfLife.set_rules(r_cornway);
                zoom = gameOfLife.get_grid()->get_auto_zoom_factor();
            }
            if(example_flags[1]){
                uiOptionsFlag |= UI_Options_Flag_enable_AutoScroll | UI_Options_Flag_enable_AutoZoom;
                //Figure *f = new Figure(lws, lws_dim);
                std::string lws_string = "b4o$o3bo$4bo$o2b1o!";
                Vec2 lws_dim = {5, 4};
                auto *f = new Figure(lws_string, lws_dim);
                gameOfLife.populate_figure(f,(Vec2){x_pos,y_pos}, angle/90);
                gameOfLife.set_rules(r_cornway);
                zoom = gameOfLife.get_grid()->get_auto_zoom_factor();

            }
            if(example_flags[2]){
                uiOptionsFlag |= UI_Options_Flag_enable_AutoScroll | UI_Options_Flag_enable_AutoZoom;
                //Figure *f = new Figure(mws, mws_dim);
                std::string mws_string = "b5o$o4bo$5bo$o3bob$2bo3b!";
                Vec2 mws_dim = {6, 5};
                auto *f = new Figure(mws_string, mws_dim);
                gameOfLife.populate_figure(f,(Vec2){x_pos,y_pos}, angle/90);
                gameOfLife.set_rules(r_cornway);
                zoom = gameOfLife.get_grid()->get_auto_zoom_factor();

            }
            if(example_flags[3]){
                uiOptionsFlag |= UI_Options_Flag_enable_AutoScroll | UI_Options_Flag_enable_AutoZoom;

                //Figure *f = new Figure(hws, hws_dim);
                std::string hws_string = "b6o$o5bo$6bo$o4bob$2b2o2b!";
                Vec2 hws_dim = {7, 5};
                auto *f = new Figure(hws_string, hws_dim);
                gameOfLife.populate_figure(f,(Vec2){x_pos,y_pos}, angle/90);
                gameOfLife.set_rules(r_cornway);
                zoom = gameOfLife.get_grid()->get_auto_zoom_factor();
            }
            if(example_flags[4]){
                uiOptionsFlag |= UI_Options_Flag_enable_AutoScroll | UI_Options_Flag_enable_AutoZoom;

                Vec2 gosper_dim = {36, 9};
                std::string gosper_string = "24bo$22bobo$12b2o6b2o12b2o$11bo3bo4b2o12b2o$2o8bo5bo3b2o$2o8bo3bob2o4bobo$10bo5bo7bo$11bo3bo$12b2o!";

                auto *f = new Figure(gosper_string, gosper_dim);
                gameOfLife.populate_figure(f,(Vec2){x_pos,y_pos}, angle/90);
                gameOfLife.set_rules(r_cornway);
                zoom = gameOfLife.get_grid()->get_auto_zoom_factor();
            }
            if(example_flags[5]){
                uiOptionsFlag |= UI_Options_Flag_enable_AutoScroll | UI_Options_Flag_enable_AutoZoom;
                //Figure *f = new Figure(eater, eater_dim);
                std::string eater_string = "2o2b$obob$2bob$2b2o!";
                Vec2 eater_dim = {4, 4};
                auto *f = new Figure(eater_string, eater_dim);
                gameOfLife.populate_figure(f,(Vec2){x_pos,y_pos}, angle/90);
                gameOfLife.set_rules(r_cornway);
                zoom = gameOfLife.get_grid()->get_auto_zoom_factor();
            }
            if(example_flags[6]){
                uiOptionsFlag |= UI_Options_Flag_enable_AutoScroll | UI_Options_Flag_enable_AutoZoom;
                //Figure *f = new Figure(htw, htw_dim);
                std::string htw_string = "10o30b10o$10o30b10o$10o30b10o$10o30b10o$10o30b10o$10o30b10o$10o30b10o$10o30b10o$10o30b10o$10o30b10o$10b11o19b20o10b10o10b10o10b10o$10b14o16b20o10b10o10b10o10b10o$10b16o14b20o10b10o10b10o10b10o$10b17o13b20o10b10o10b10o10b10o$10b18o12b20o10b10o10b10o10b10o$10b18o12b20o10b10o10b10o10b10o$10b19o11b20o10b10o10b10o10b10o$10b19o11b20o10b10o10b10o10b10o$10b19o11b20o10b10o10b10o10b10o$10b20o10b20o10b10o10b10o10b10o$10o10b10o10b10o20b10o10b10o10b10o$10o10b10o10b10o20b10o10b10o10b10o$10o10b10o10b10o20b10o10b10o10b10o$10o10b10o10b10o20b10o10b10o10b10o$10o10b10o10b10o20b10o10b10o10b10o$10o10b10o10b10o20b10o10b10o10b10o$10o10b10o10b10o20b10o10b10o10b10o$10o10b10o10b10o20b10o10b10o10b10o$10o10b10o10b10o20b10o10b10o10b10o$10o10b10o10b10o20b10o10b10o10b10o$10o10b10o10b20o10b40o10b10o$10o10b10o11b19o11b39o10b10o$10o10b10o11b19o11b39o10b10o$10o10b10o11b19o11b39o10b10o$10o10b10o12b18o12b38o10b10o$10o10b10o12b18o12b38o10b10o$10o10b10o13b17o13b37o10b10o$10o10b10o14b16o14b36o10b10o$10o10b10o16b14o16b34o10b10o$10o10b10o19b11o19b31o10b10o!";
                Vec2 htw_dim = {130, 40};
                auto *f = new Figure(htw_string, htw_dim);
                gameOfLife.populate_figure(f,(Vec2){x_pos,y_pos}, angle/90);
                gameOfLife.set_rules(r_copy);
                zoom = gameOfLife.get_grid()->get_auto_zoom_factor();
            }
            if(example_flags[7]){
                uiOptionsFlag |= UI_Options_Flag_enable_AutoScroll | UI_Options_Flag_enable_AutoZoom;
                Vec2 pacman_dim = {13, 13};
                //Figure *f = new Figure(pacman, pacman_dim);
                std::string pacman_string = "5b3o$3b2o3b2o$2bo7bo$bo4b3o2bo$bo4b3o2bo$o11bo$o5b7o$o6bo$bo6bo$bo7bo$2bo7bo$3b2o3b2o$5b3o!";
                auto *f = new Figure(pacman_string, pacman_dim);
                gameOfLife.populate_figure(f,(Vec2){x_pos,y_pos}, angle/90);
                gameOfLife.set_rules(r_copy);
                zoom = gameOfLife.get_grid()->get_auto_zoom_factor();
            }

            for(auto c: gameOfLife.get_grid()->get_cells()){
                c->get_neighbours()->set_n_alive(0);
                for(auto n: c->get_neighbours()->get_cells()){
                    if(n->get_state()){ c->get_neighbours()->set_n_alive(c->get_neighbours()->get_n_alive()+1); }
                }
                c->set_color(gameOfLife.get_color_rules()->color[c->get_neighbours()->get_n_alive()]);
            }
            reset_figure = Figure(&gameOfLife);
            uiFlag &= ~UI_Flag_enable_LoadExampleUI;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
        // abort button
        ImGui::SameLine();
        //color red
        ImGui::PushID(0);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.8f));
        if (ImGui::Button("Abort")) {
            uiFlag &= ~UI_Flag_enable_LoadExampleUI;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        ImGui::End();
    }
    //=================================================================================================== CLASS UI ===//

} // gol