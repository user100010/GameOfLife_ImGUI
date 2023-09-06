//
// Created by
//          Benjamin Grothe
// on 06.05.23
// all Copyrights included by Benjamin Grothe
//

#include <utility>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <string>

#include "../ImGUI/imgui.h"
#include "../ImGUI/backends/imgui_impl_sdl2.h"
#include "../ImGUI/backends/imgui_impl_opengl2.h"
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>

#ifdef _Win32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#define STACK_SIZE      100

namespace gol {
    class Cell;
    class Neighbours;
    class Grid;
    class Figure;
    class GameOfLife;
    class UI;

    // TODO: zoom grid to fit in Grid UI (for exp. Fullscreen)

    // TODO: increase example speed ???

    // TODO: ERROR HANDLING {get rule from .lif if exist}
    // TODO: optimize optical User Interface setup with ImGui


    // FIXME: irgendwas stimmt mit dem save_button nicht !!!! random -> screenshot -> speichert nur 300x300 und nicht 475x300 <- problem liegt nur bei dimensions_to_string

    // NOTE: actually only 16bit (max ca. 180x180 cells) !? should be 32bit but how -> uncomment << #define ImDrawIdx unsigned int >> in imconfig.h
    // NOTE: 32Bit works for MacOS X & Linux --> windows not testet

    /**
     * @brief includes Figures and is used for step back <br> Stack size is defined by STACK_SIZE
     */
    typedef std::list<Figure*> Figure_Stack;

    /** @brief UI_Flags includes information if UI Windows are activated or not
     *  @note Flag values are defined in enumeration UI_Flag_
     **/
    typedef unsigned int UI_Flag;
    /**
     * @brief UI_Flag_ defines Flags for UI_Flag typedef;
     */
    enum UI_Flag_{
        UI_Flag_none = 0,                       // no UI_Flag is set
        UI_Flag_enable_GridUI = 1<<0,           // UI_Flag which is used for enabling Grid_UI
        UI_Flag_enable_SetupUI = 1<<1,          // UI_Flag which is used for enabling Setting_UI
        UI_Flag_enable_LoadUI = 1<<2,           // UI_Flag which is used for enabling Load_UI
        UI_Flag_enable_LoadExampleUI = 1<<3,    // UI_Flag which is used for enabling Load_Exp_UI
        UI_Flag_enable_SaveUI = 1<<4,           // UI_Flag which is used for enabling Save_UI
        UI_Flag_enable_MessageUI = 1<<5         // UI_Flag which is used for enabling Message_UI
    };

    /**
     * @brief UI_Options_Flag includes some informations about some UI options
     * @note Flag values are defined in enumeration UI_Options_Flag_
     */
    typedef unsigned int UI_Options_Flag;
    /** @brief UI_Options_Flag_ defines Flags for UI_Options_Flag typedef */
    enum UI_Options_Flag_{
        UI_Options_Flag_none = 0,                   // no UI_Options_Flag is set
        UI_Options_Flag_Run = 1<<0,                 // UI_Options_Flag which shows if Game of life is running
        UI_Options_Flag_enable_AutoZoom = 1<<2,     // UI_Options_Flag which enabling auto zoom
        UI_Options_Flag_enable_AutoScroll = 1<<3,   // UI_Options_Flag which enabling auto scroll
        UI_Options_Flag_enable_ColorizeAll = 1<<4   // UI_Options_Flag which shows if all alive cells have the same color
    };

    /** @brief structure defines a 2 dimensional Vector*/
    struct Vec2 { int x, y; };                      // 2 dimensional Vector
    /**
     * @brief structure for Game of Life Rules.
     * @brief shows what to do if n neighbour cells are alive or dead
     **/
    struct Rule { bool alive[9], death[9]; };
    /**
     * @brief structure for Game of Life Color Rules
     * @brief shows which cell color should be used if n neighbour cells are alive
     **/
    struct RuleColor { unsigned int color[9]; };

    //================================================================================================= CLASS CELL ===//
    /**
     * @file game_of_life.cpp
     * @brief defines a Cell (smallest element in grid)
     */
    class Cell{
    private:
        Vec2 position;                      // position in grid
        bool state;                         // shows if this cell is alive [true] or death [false]
        unsigned int color;                 // includes cell Color
        Grid* grid;                         // class grid which includes this cell
        Neighbours* neighbours;             // class neighbours includes all Cells next to this cell

    public:
        /**
         * @brief Constructor for object cell
         * @param _grid     : gol::Grid     -> owner grid
         * @param _position : gol::Vec2     -> position in grid
         */
        Cell(Grid* _grid, Vec2 _position);

        /** @brief Destructor for object cell */
        ~Cell() = default;

        /**
         * @brief getter function for private member position
         * @return position of cell in grid as const gol::Vec2
         */
        Vec2 get_position() const { return position; }
        /**
         * @brief getter function for private member state
         * @return shows if cell is alive or dead as const bool
         */
        bool get_state() const { return state; }
        /**
         * @brief getter function for private member color
         * @return color of cell as const unsigned int
         */
        unsigned int get_color() const {return color; };
        /**
         * @brief getter function for private member grid
         * @return owner grid as gol::Grid*
         */
        Grid* get_grid() const { return grid; }
        /**
         * @brief getter function for private member neighbours
         * @return neighbour as gol::Neighbours*
         */
        Neighbours* get_neighbours() const { return neighbours; }

        //void set_position(Vec2 _position){ position = _position; }
        /**
         * @brief setter function for private member state
         * @param _state : bool -> new state [true = alive, false = dead]
         */
        void set_state(bool _state){ state = _state; }
        /**
         * @brief setter function for private member color
         * @param _color : unsigned int -> set new cell color
         */
        void set_color(unsigned int _color){ color = _color; }
        /**
         * @brief setter function for private member neighbours
         * @param _neighbours : gol::Neighbours* -> set new Neighbours Object which includes all neighbour cells
         */
        void set_neighbours(Neighbours* _neighbours){ neighbours = _neighbours; }

    };
    //================================================================================================= CLASS GRID ===//
    /**
     * @file game_of_life.cpp
     * @brief object define a Grid which includes x*y cells
     */
    class Grid{
    private:
        Vec2 dimension;             // dimension of this grid
        std::vector<Cell*> cells;   // includes all existing cells in this grid
        GameOfLife* gameOfLife;     // owner GameOfLife object [unused]

        /**
         * @brief calculate the position of left upper corner of real_grid_dimension "Figure"
         * @return gol::Vec2
         **/
        Vec2 get_real_grid_position();

    public:
        /**
         * @brief constuctor for object grid
         * @param _gameOfLife   : gol::GameOfLife   -> owner GameOfLife
         * @param _dimension    : gol::Vec2         -> dimensions of this grid
         */
        Grid(GameOfLife* _gameOfLife, Vec2 _dimension);
        /** @brief Desturctor for object grid */
        ~Grid() = default;

        Vec2 get_dimension() const { return dimension; }
        /**
         * @brief return all cells in this grid
         * @return all cells in grid as std::vector<gol::Cell*>
         */
        std::vector<Cell*> get_cells() { return cells;}
        /**
         * @brief return explicit cell
         * @param position  : gol::Vec2     -> position of cell in grid
         * @return single cell on position as gol::Cell
         */
        Cell* get_cell(Vec2 position) { return cells[((position.y* this->dimension.x)+position.x)]; }
        //GameOfLife* get_gameOfLife(){ return gameOfLife; }

        /**
         * @brief calculate dimension of Grid with alive cells,
         * @brief the borders (up,down,left,right) with dead cells will be cut
         * @return gol::Vec2
         **/
        Vec2 get_real_grid_dimension();
        /**
         * @brief calculate x-axis scroll position which will be used in autoscroll mode by Grid_UI
         * @return float
         */
        float get_scrollx_position();
        /**
         * @brief calculate y-axis scroll position which will be used in autoscroll mode by Grid_UI
         * @return float
         */
        float get_scrolly_position();
        /**
         * @brief calculate autozoom factor which will be used by Grid_UI
         * @return float
         */
        float get_auto_zoom_factor();

    };
    //=========================================================================================== CLASS NEIGHBOURS ===//
    /**
     *  @file game_of_life.cpp
     * @brief object define all neighbour Cells of owner cells
     */
    class Neighbours{
    private:
        Cell* owner;                // cell which owns this object
        std::vector<Cell*> cells;   // includes all cells next to owner

        int n_alive;                // shows number of cells with state true

    public:
        /**
         * @brief constructor of object Neighbours
         * @param _owner : gol::Cell -> cell which owns this object
         */
        explicit Neighbours(Cell* _owner);
        /** @brief Desturctor for object Neighbours */
        ~Neighbours() = default;

        /**
         * @brief getter function for all neighbour cells from owner
         * @return all neighbour cells from owner as std::vector<Cell*>
         */
        std::vector<Cell*> get_cells() const { return cells; }
        //Cell* get_owner() { return owner; }
        /**
         * @brief getter function for private member n_alive
         * @return number of all alive cells as int
         */
        int get_n_alive() const { return n_alive; }
        /**
         * @brief setter function for private member n_alive
         * @param _n : int -> new value of all alive cells
         */
        void set_n_alive(int _n) { n_alive = _n; }
    };
    //=============================================================================================== CLASS FIGURE ===//
    /**
     *  @file game_of_life.cpp
     * @brief object which define a Figure with cell states
     * @brief is used to load into grid
     */
    class Figure{
    private:
        Vec2 dimensions;            // dimensions of this object
        std::vector<bool> states;   // all states of this object

        /**
         * @brief filter dimensions with format like "x=100 , y=150" from std::string
         * @param str : std::string
         * @return dimensions as gol::Vec2
         */
        static Vec2 convert_string_to_dimensions(std::string str);
        /**
         * @brief convert a string in .lif format to an boolean vector
         * @note used to load figure from file
         * @param str : std::string
         * @return std::vector<bool>
         */
        std::vector<bool> convert_string_to_states(std::string str);

        /** @brief convert the real figure dimensions to string */
        std::string convert_dimension_to_string();
        /** @brief convert states to a single string in .lif format */
        std::string convert_states_to_string();



    public:
        /**
         * @brief explict Constructor for Figure object
         * @param _states : std::vector<bool> -> include all new Figure cell states
         * @param _dimensions  : gol::Vec2 -> includes dimension of new Figure object
         */
        explicit Figure(std::vector<bool> _states, Vec2 &_dimensions){dimensions = _dimensions; states = std::move(_states);  }
        /**
         * @brief explict Constructor for Figure object
         * @param str : std::string& -> string in .lif format which will be converted into private member states
         * @param _dimensions : gol::Vec2 -> includes dimension of new Figure object
         */
        explicit Figure(std::string &str, Vec2 &_dimensions){ dimensions = _dimensions; states = convert_string_to_states(str); }
        /**
         * @brief constructor which loads object figure from .lif file
         * @param path : std::string    -> full path to .lif file
         */
        explicit Figure(std::string &path);
        /**
         * @brief constructor to save Game Of Lifes grid into a figure
         * @param gameOfLife : gol::GameOfLife
         */
        explicit Figure(GameOfLife* gameOfLife);
        /** @brief Desturctor for object Figure */
        ~Figure() = default;

        /**
         * @brief getter function for private member dimension
         * @return dimension of figure as gol::Vec2
         */
        Vec2 get_dimension() const { return dimensions; }
        //std::vector<bool> get_states() const { return states; } // unused
        /**
         * @brief getter function for explicit state in private member states
         * @param _position : gol::Vec2 position of state in
         * @return
         */
        bool get_state(Vec2 _position) const { return states[(_position.y*dimensions.x)+_position.x]; }

        /**
         * @brief save all states of this object into a .lif file
         * @param path  : std::string   -> full filepath of new .lif file
         * @return for Error handling
         */
        int save_to_file(std::string &path);

    };
    //========================================================================================= CLASS GAME OF LIFE ===//
    /**
     *  @file game_of_life.cpp
     * @brief object which include a grid and is used to run Game Of Life with user interface
     */
    class GameOfLife{
    private:
        Grid* grid;                 // grid object with cells
        Rule rules;                 // includes rules for game of life
        RuleColor rules_color;      // includes color rules for game of life


        /**
         * @brief check neighbours in Cell and return new state for cell
         * @param _cell : Cell*
         * @return new state as bool;
         */
        bool check_rules_in_cell(Cell* _cell);

        /** @brief check rules for every Cell in Grid and refresh Grid after checking rules */
        void check_rules_in_grid();

        /** @returns random boolean value (50:50) */
        static bool random_bool();

    public:
        /**
         * @brief constructor of GameOfLife object
         * @param _dimensions   : gol::Vec2     -> dimensions of grid in this object
         * @param _rules        : gol::Rules
         */
        GameOfLife(Vec2 _dimensions, Rule _rules, RuleColor _rulesColor);
        /** @brief Destructor of GameOfLife object */
        ~GameOfLife() = default;

        Grid* get_grid(){ return grid; }
        Rule* get_rules(){ return &rules; }
        RuleColor* get_color_rules(){ return &rules_color; }

        void set_rules(Rule rule){ rules = rule; }

        /**
         * @brief MASTER-function to run game of life with ImGui interface.
         * @return for error handling
         */
        int runUI();
        /**
         * @brief populate grid in this object with informations from figure object
         * @param _figure   : gol::Figure
         * @param position  : gol::Vec2     -> position of figure in grid
         * @param angle     : Int           -> 0=[0°];1=[90°];2=[180°];3=[270°]
         */
        void populate_figure(Figure* _figure, Vec2 position, int angle);
        /** @brief populate grid object with random cell states */
        void populate_random();
        /** @brief check all cells grid and refresh grid object*/
        void refresh_grid(){ check_rules_in_grid(); } // FIXME: its to slow ( ca  << 9(x*y)++ >> loops on 1 core ) <- Optimazion flag -O1 !! <- eventually save only alive cells in grid calculate neighbours and change them "Stack style"
        /** @brief kill all cells in grid object */
        void clear_grid();


    };
    //=================================================================================================== CLASS UI ===//
    /**
     *  @file game_of_life.cpp
     * @brief object which includes all Buttons an UI functions << used by gol::GameOfLife::run() >> */
    class UI{
    private:

        /**
         * @brief print rectangles with size of zoom << used by gol::UI::grid_UI() >>
         * @param _gameOfLife   : &GameOfLife   -> includes grid & cell information
         * @param zoom          : float         -> zoom factor
         */
        static void print(GameOfLife* _gameOfLife, float zoom);
        /**
         * @brief create zoom in/out buttons << used by gol::UI::setting_UI() >>
         * @param factor    : &float  -> zoom factor
         * @param uiOptionsFlag : &UI_Options_Flag -> includes auto zoom enabled option flag
         */
        static void button_setting_zoom(UI_Options_Flag &uiOptionsFlag, float &factor);
        /**
         * @brief create speed up/down buttons << used by gol::UI::setting_UI() >>
         * @param factor        : &int  -> speed factor
         */
        static void button_setting_speed(int &factor);
        /**
         * @brief create step for/back buttons << used by gol::UI::setting_UI() >>
         * @param gameOfLife    : &GameOfLife   -> includes grid & cell information
         * @param _stack        : &Figure_Stack -> includes STACK_SIZE pre Figures for reverse step
         */
        static void button_setting_step(GameOfLife &gameOfLife, Figure_Stack &_stack);
        /**
         * @brief create run button << used by gol::UI::setting_UI() >>
         * @param uiOptionsFlag : &UI_Options_Flag -> includes run option flag
         */
        static void button_setting_run(UI_Options_Flag &uiOptionsFlag);
        /**
         * @brief create stop button << used by gol::UI::setting_UI() >>
         * @param uiOptionsFlag : &UI_Options_Flag -> includes run option flag
         */
        //static void button_setting_stop(UI_Options_Flag &uiOptionsFlag);
        /**
         * @brief Create reset button << used by gol::UI::setting_UI() >>
         * @param gameOfLife    : &GameOfLife   -> includes grid & cell information
         * @param reset_figure  : &Figure       -> figure which includes reset grid states
         * @param figureStack   : &Figure_Stack -> STACK_SIZE pre Figures for reverse step
         */
        static void button_setting_reset(GameOfLife &gameOfLife, Figure &reset_figure, Figure_Stack &figureStack);
        /**
         * @brief Create clear button << used by gol::UI::setting_UI() >>
         * @param gameOfLife    : &GameOfLife   -> includes grid & cell information
         * @param reset_figure  : &Figure       -> figure which includes reset grid states
         * @param figureStack   : &Figure_Stack -> STACK_SIZE pre Figures for reverse step
         */
        static void button_setting_clear(GameOfLife &gameOfLife, Figure &reset_figure, Figure_Stack &figureStack);
        /**
         * @brief create random button << used by gol::UI::setting_UI() >>
         * @param gameOfLife     : &GameOfLife   -> includes grid & cell information
         * @param reset_figure   : &Figure       -> figure which includes reset grid states
         * @param figureStack    : &Figure_Stack -> STACK_SIZE pre Figures for reverse step
         */
        static void button_setting_random(GameOfLife &gameOfLife, Figure &reset_figure, Figure_Stack &figureStack);
        /**
         * @brief create load button << used by gol::UI::setting_UI() >>
         * @param uiFlag    : &UI_Flag -> includes load_ui flag to enable/disable window
         */
        static void button_setting_load(UI_Flag &uiFlag);//TODO
        /**
         * @brief create example load button << used by gol::UI::setting_UI() >>
         * @param uiFlag    : &UI_Flag -> includes load_exp_ui flag to enable/disable window
         */
        static void button_setting_load_exp(UI_Flag &uiFlag);
        /**
         * @brief create screenshot button << used by gol::UI::setting_UI() >>
         * @param uiFlag    : &UI_Flag -> includes ui flag  to enable/disable window
         */
        static void button_setting_screenshot(UI_Flag &uiFlag);
        /**
         * @brief create reset rules button << used by gol::UI::setting_UI() >>
         * @param _rule     : *Rule -> rules which will be reset to Cornways Game of life [23/3]
         */
        static void button_setting_reset_rules(Rule* _rule);
        /**
         * @brief create reset colors button << used by gol::UI::setting_UI() >>
         * @param _rule     : *RuleColor    -> rules which will be reset to all white
         */
        static void button_setting_reset_colors(RuleColor* _rule);
        /**
         * @brief create checkboxes for rule and color modifications << used by gol::UI::setting_UI() >>
         * @param uiOptionsFlag : &UI_Options_Flag -> includes color all option flag
         * @param gameOfLife    : &GameOfLife   -> includes grid & cell information
         */
        static void setup_rules(UI_Options_Flag &uiOptionsFlag, GameOfLife &gameOfLife);
        /**
         * @brief creates load button to load figure from file into grid << used by gol::UI::load_UI() >>
         * @param uiFlag        : &UI_Flag          -> includes load_UI & message_UI flags to enable/disable window
         * @param uiOptionsFlag : &UI_Options_Flag  -> used to enable auto scroll and auto zoom
         * @param gameOfLife    : &GameOfLife       -> includes grid & cell information
         * @param reset_figure  : &Figure           -> figure which includes reset grid states
         * @param path          : std::string       -> full filepath to .lif data
         * @param position      : Vec2              -> position upper left corner of Figure in grid
         * @param angle         : &int              -> angle of figure in grid << 0=0°,1=90°,2=180°,3=270° >>
         * @param message       : &std::string      -> error message << used by gol::UI::message_UI() >>
         */
        static void button_load(UI_Flag &uiFlag, UI_Options_Flag &uiOptionsFlag, GameOfLife &gameOfLife, Figure &reset_figure, std::string path, Vec2 position, int &angle, std::string &message, float &zoom);
        /**
         * @brief creates save button to save figure into a file from grid << used by gol::UI::save_UI() >>
         * @param uiFlag        : &UI_Flag      -> includes save_UI flags to enable/disable window
         * @param gameOfLife    : &GameOfLife   -> includes grid & cell information
         * @param path          : std::string   -> full filepath to new .lif data
         */
        static void button_save(UI_Flag &uiFlag, GameOfLife &gameOfLife, std::string path);

    public:
        /**
         * @brief initialize Grid user interface
         * @param uiFlag        : &UI_Flag          -> includes Grid_UI & setting_UI flags to enable/disable window
         * @param uiOptionsFlag : &UI_Options_Flag  -> used to enable auto scroll, auto zoom and run
         * @param gameOfLife    : gol::GameOfLife   -> includes grid & cell information
         * @param zoom          : &float            -> zoom factor
         * @param speed         : &int              -> speed factor
         * @param count         : &int              -> counter required by speed
         */
        static void grid_UI(UI_Flag &uiFlag, UI_Options_Flag &uiOptionsFlag, GameOfLife &gameOfLife, Figure_Stack &figureStack, float &zoom, int &speed, int &count);
        /**
         * @brief initialize Setting user interface
         * @param uiFlag        : &UI_Flag          -> includes UI flags to enable/disable window
         * @param uiOptionsFlag : &UI_Options_Flag  -> used to enable auto scroll, auto zoom and run
         * @param gameOfLife    : gol::GameOfLife   -> includes grid & cell information
         * @param reset_figure  : gol::Figure       -> reference required for resetting grid
         * @param zoom          : float             -> zoom factor
         * @param speed         : int               -> speed factor
         * @param run           : bool              -> run Game of Life trigger
         * @param load_UI       : bool              -> load user interface trigger
         * @param save_UI       : bool              -> save user interface trigger
         */
        static void setting_UI(UI_Flag &uiFlag, UI_Options_Flag &uiOptionsFlag, GameOfLife &gameOfLife, Figure &reset_figure, Figure_Stack &figureStack,float &zoom, int &speed);
        /**
         * @brief initialize Load user interface
         * @param uiFlag        : &UI_Flag          -> includes UI flags to enable/disable window
         * @param uiOptionsFlag : &UI_Options_Flag  -> used to enable auto scroll, auto zoom
         * @param gameOfLife    : gol::GameOfLife   -> includes grid & cell information
         * @param reset_figure  : gol::Figure       -> reference required for resetting grid
         * @param load_UI       : bool              -> load user interface trigger
         * @param message_UI    : bool              -> message user interface trigger
         * @param message       : std::string       -> required for error handling
         */
        static void load_UI(UI_Flag &uiFlag, UI_Options_Flag &uiOptionsFlag, GameOfLife &gameOfLife, Figure &reset_figure, std::string &message, float &zoom);
        /**
         * @brief initialize Screenshot user interface
         * @param uiFlag        : &UI_Flag          -> includes save_UI flags to enable/disable window
         * @param gameOfLife    : gol::GameOfLife   -> includes grid & cell information
         * @param message       : std::string       -> required for status messages
         */
        static void screenshot_UI(UI_Flag &uiFlag, GameOfLife &gameOfLife, std::string &message);
        /**
         * @brief initialize MessageBox user interface
         * @param uiFlag        : &UI_Flag      -> includes UI flags  to enable/disable window
         * @param message       : std::string   -> message to display
         */
        static void messageBox_UI(UI_Flag &uiFlag, std::string &message);
        /**
         * @brief creates colorized rectangle with color picker function
         * @param gameOfLife    : gol::GameOfLife   -> includes grid & cell information
         * @param label         : const char*       -> label of rectangle
         * @param color         : unsigned integer* -> color of rectangle
         */
        static void ColorPicker(GameOfLife &gameOfLife, const char* label, ImU32 *color);

        /**
         * @brief initialize load example user interface
         * @param uiFlag        : &UI_Flag          -> includes UI Flags to enable/disable window
         * @param uiOptionsFlag : &UI_Options_Flag  -> used to enable auto scroll, auto zoom
         * @param gameOfLife    : gol::GameOfLife   -> includes grid & cell information
         * @param reset_figure  : gol::Figure       -> reference required for resetting grid
         * @param zoom          : &float            -> zoom factor
         * @param speed         : &int              -> speed factor
         */
        static void load_example_UI(UI_Flag &uiFlag, UI_Options_Flag &uiOptionsFlag, GameOfLife &gameOfLife, Figure &reset_figure, float &zoom, int &speed);

        /**
         * @brief initialize Dockspace Window for user interface windows like Grid_UI or Setting_UI
         */
        static void Dockspace_UI();
    };
} // gol

