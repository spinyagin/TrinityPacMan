#pragma once
#include "Define.h"
#include "GameObjectData.h"

namespace PacMan {
    static constexpr uint32 AT_PLAYER_START = 6000;
    static constexpr uint32 AT_NPC_VANTAGE = 6001;

    static constexpr uint32 PC_GOSSIP_STOP_EVENT = 1;
    static constexpr uint32 PC_GOSSIP_SET_DEADLINE = 2;
    static constexpr uint32 PC_GOSSIP_DEBUG_IDLE_GHOSTS = 3;
    static constexpr uint32 PC_GOSSIP_DUMMY = 100;

    static constexpr Milliseconds EVENT_DEADLINE = 240s;

    static constexpr Milliseconds PLAYER_STUN_DURATION = 5100ms;
    static constexpr Milliseconds EVENT_AURA_STACK_SPEED = 25s; // the duration of the aura is 30 seconds
    static constexpr Milliseconds GHOST_AI_SCATTER_TIME = 7s;  // pac-man original 7 seconds
    static constexpr Milliseconds GHOST_AI_CHASE_TIME = 15s;  // pac-man original 20 seconds
    static constexpr Milliseconds GHOST_AI_FRIGHTENED_TIME = 10s;
    static constexpr Milliseconds BLINKY_AI_DELAYED_START = 5s;
    static constexpr Milliseconds FRUIT_LIFETIME = 15s;

    static constexpr Milliseconds GHOST_CORPSE_TELEPORT_TIME = 1500ms;
    static constexpr Milliseconds GHOST_RESPAWN_TIME = 5s;

    static constexpr int32_t CLYDE_AI_CHASE_DISTANCE = 8;

    /* used if ghosts start by timer */
    static constexpr Milliseconds INKY_AI_DELAYED_START = 10s;
    static constexpr Milliseconds CLYDE_AI_DELAYED_START = 15s;
    static constexpr bool GHOSTS_START_BY_TIMER = true;

    /* scatter positions */
    static constexpr std::pair<int32_t, int32_t> BLINKY_SCATTER_POS1 = { 1, 21 };
    static constexpr std::pair<int32_t, int32_t> BLINKY_SCATTER_POS2 = { 4, 20 };
    static constexpr std::pair<int32_t, int32_t> BLINKY_SCATTER_POS3 = { 2, 17 };

    static constexpr std::pair<int32_t, int32_t> INKY_SCATTER_POS1 = { 23, 21 };
    static constexpr std::pair<int32_t, int32_t> INKY_SCATTER_POS2 = { 18, 17 };
    static constexpr std::pair<int32_t, int32_t> INKY_SCATTER_POS3 = { 22, 12 };

    static constexpr std::pair<int32_t, int32_t> CLYDE_SCATTER_POS1 = { 23, 1 };
    static constexpr std::pair<int32_t, int32_t> CLYDE_SCATTER_POS2 = { 18, 5 };
    static constexpr std::pair<int32_t, int32_t> CLYDE_SCATTER_POS3 = { 22, 10 };

    /* scores define */
    static constexpr uint32_t SCORE_REGULAR_POINT = 10;
    static constexpr uint32_t SCORE_POWER_POINT = 50;
    static constexpr uint32_t SCORE_FRUIT_POINT = 100;
    static constexpr uint32_t SCORE_GHOST_EATEN = 200;


    /* ghost phrases */
    static const char* GHOSTS_SAYS_WHEN_ANOTHER_ONE_IS_EATEN[10] = {
        "Oh no! %s got caught!",
        "Looks like %s was too slow!",
        "%s, you fell for it!",
        "Stay sharp, %s!",
        "Careful next time, %s!",
        "Keep an eye out, %s!",
        "You'll get 'em next time, %s!",
        "Almost had 'em, %s!",
        "%s, better luck next round!",
        "He is too fast for us, %s!"
    };

    static const char* GHOSTS_SAYS_WHEN_ANOTHER_ONE_CATCH_PLAYER[10] = {
        "Wow, %s got one!",
        "%s caught a snack!",
        "Nice catch, %s!",
        "Great job, %s!",
        "%s got 'em!",
        "Way to go, %s!",
        "Good move, %s!",
        "That's how it's done, %s!",
        "Impressive, %s!",
        "You got 'em, %s!"
    };

    static const char* GHOST_SAYS_WHEN_WAS_EATEN[10] = {
        "Nice one, %s! But I'll be back!",
        "You got lucky, %s! Watch your back!",
        "Good move, %s! But I'll get you next time!",
        "You're fast, %s! But I'll catch up!",
        "You're good, %s! But I'm not done yet!",
        "Impressive, %s! But I'll haunt you again!",
        "You're quick, %s! But I'll return!",
        "Well played, %s! But I'll be waiting!",
        "Great job, %s! But I'll catch you later!",
        "You won this round, %s! But I'll be back for more!"
    };

    static const char* GHOST_SAY_WHEN_WAS_FRIGHTENED[10] = {
        "Ahh! %s is scary!",
        "I'm outta here! %s is scary!",
        "I'm not sticking around! %s is scary!",
        "I'm not waiting around! %s is scary!",
        "Ahh! %s is chilling!",
        "I'm outta here! %s is spine-tingling!",
        "I'm not lingering! %s is bone-chilling!",
        "I'm not pausing! %s is hair-raising!",
        "I'm not loitering! %s is giving me goosebumps!",
        "I'm not stopping! %s is eerie!"
    };

//     static constexpr uint32 AT_LEFT_TP_POINT = 6002;
//     static constexpr uint32 AT_RIGHT_TP_POINT = 6003;

/* The map can be loaded in any other way and can be binary for optimal calculations, but for example it is hardcoded and human readable */
    constexpr const char* TXT_MAP =
R"(XXXXXXXXXXXXXXXXXXXXXXX
X1111111111X1111111111X
X1XXX1XXXX1X1XXXX1XXX1X
X2XXX1XXXX1X1XXXX1XXX2X
X111111111111111111111X
X1XXX1X1XXXXXXX1X1XXX1X
X11111X1XXXXXXX1X11111X
XXXXX1X1111X1111X1XXXXX
xxxxX1XXXX0X0XXXX1Xxxxx
xxxxX1X3333B3333X1Xxxxx
XXXXX1X3XXX0XXX3X1XXXXX
LLr00103X0I0c0X30100lRR
XXXXX1X3XXXXXXX3X1XXXXX
xxxxX1X333333333X1Xxxxx
xxxxX1X0XXXXXXX0X1Xxxxx
XXXXX1X0XXXXXXX0X1XXXXX
X1111111111X1111111111X
X1XXX1XXXX1X1XXXX1XXX1X
X211X111111C111111X112X
XXX1X1X1XXXXXXX1X1X1XXX
X11111X1111X1111X11111X
X1XXXXXXXX1X1XXXXXXXX1X
X1XXXXXXXX1X1XXXXXXXX1X
X111111111111111111111X
XXXXXXXXXXXXXXXXXXXXXXX
xxxxxxxxxxxxxxxxxxxxxxx
xPxPxPxxxxxxxxxxxxxFxFx)";

/* must match the quantity on the map*/
static constexpr uint32_t PACMAN_FRUIT_COUNTER = 2;
static constexpr uint32_t PACMAN_LIFE_COUNTER = 3;

// X - wall
// x - unaccessible empty cell
// 0 - empty cell
// C - player start position
// B, I, c - Blinky, Inky, Clyde start position
// L - left teleport point
// R - right teleport point
// l - left teleport target
// r - right teleport target
// P - player life counter
// F - eaten fruits counter
// 3 - possible fruit position

    /* simple structure instead class Position */
    struct SSquare {
        struct SPosition {
            float x;
            float y;
            float z;
        };

        SPosition left_top;
        SPosition right_top;
        SPosition right_bottom;
        SPosition left_bottom;
    };

    struct STargetPosition {
        QuaternionData quanterion;
        Position position;
    };

    static constexpr SSquare POS_MAP = {
        {7060.105957f, 4380.169922f, 872.062866f},
        {7111.892090f, 4367.100098f, 872.333069f},
        {7098.489258f, 4313.977051f, 871.325684f},
        {7046.707031f, 4327.068848f, 871.056946f}
    };

    enum ECellTemplate : char
    {
        ctEMPTY_CELL = '0',
        ctWALL_CELL = 'X',
        ctUNACCESSIBLE_CELL = 'x',
        ctREGULAR_POINT_CELL = '1',
        ctBONUS_POINT_CELL = '2',
        ctPLAYER_START_CELL = 'C',
        ctMONSTER_START_CELL = 'M',
        ctLEFT_TP_POINT = 'L',
        ctRIGHT_TP_POINT = 'R',
        ctLEFT_TP_TARGET = 'l',
        ctRIGHT_TP_TARGET = 'r',
        ctBLINKY_START_CELL = 'B',
        ctINKY_START_CELL = 'I',
        ctCLYDE_START_CELL = 'c',
        ctPLAYER_LIFE_CELL = 'P',
        ctFRUIT_CELL = 'F',
        ctFRUIT_POSITION_CELL = '3'
    };

    enum class EObjectType : int32 {
        Unknown = -1,
        Creature = 0,
        GameObject = 1
    };

    enum EPacmManHandlerAction : uint16 {
        CHECK_PLAYER_POSITION = 1,
        SHOW_TELEPORT_VISUAL = 2,
        REMOVE_PLAYER_STUN = 3,
        RENEW_EVENT_AURA_DAMAGE = 4,
        RENEW_EVENT_AURA = 5,
        REMOVE_SPHERE_VISUAL = 6,
        MOVE_FRUIT = 7,
        STOP_GHOSTS = 8,

        BONUS_POINT_DESPAWN_HA = 9,

        BLINKY_WIN = 50,
        INKY_WIN = 51,
        CLYDE_WIN = 52,
        BLINKY_LOSE = 53,
        INKY_LOSE = 54,
        CLYDE_LOSE = 55,
        BLINKY = 56,
        INKY = 57,
        CLYDE = 58,
        GHOST_WIN = 59,
        GHOST_LOSE = 60,

        PLAYER_STARTED = 98,
        TELEPORT_PLAYER_AT_START = 99,
        STOP_FAIL = 100,
        STOP_SUCCESS = 101,
    };

    enum ECreatureAction : int32 {
        REGULAR_POINT_DESPAWN = 1,
        CREATURE_TOUCHED_BY_PLAYER = 2,
        REGULAR_POINT_TOUCHED_BY_PLAYER = 2,
        GHOST_DELAYED_START = 3,
        GHOST_FIND_TARGET_POINT = 4,
        GHOST_CHANGE_STATE = 5,
        GHOST_CHASE_FORCE_RECALCULATE_PATH = 6,
        GHOST_BACK_TO_START = 7,
        GHOST_RESPAWN = 8,
        BONUS_POINT_DESPAWN = 9,
        FRUIT_DESPAWN = 10,

        GET_BLINKY_POSITION = 100,

        GET_CURRENT_PLAYER = 1000,
    };

    enum class ETeleportTarget : int32 {
        Left = 0,
        Right = 1
    };

    enum ESpeells : uint32 {
        SPELL_NPC_TELEPORT_VISUAL = 73078,
        SPELL_STUN_VISUAL = 18970,
        SPELL_SELF_STUN = 80000,//48342,
        SPELL_PLAYER_TELEPORT_VISUAL = 70088,
        SPELL_EVENT_AURA = 80001,
        SPELL_EVENT_AURA_DAMAGE = 80003,
        SPELL_EVENT_AURA_DECREASE_SPEED = 80004,
        SPELL_NPC_CHANGE_STATE_VISUAL = 75459,
        SPELL_FEAR_VISUAL = 49774,
        SPELL_FEIGN_DEATH = 37493,
        SPELL_SPHERE_VISUAL = 56075,
        SPELL_CLONE_PLAYER = 45204
    };

    enum class EAIState : uint32 {
        AI_STATE_CHASE = 0,
        AI_STATE_SCATTER = 1,
        AI_STATE_FRIGHTEND = 2,
    };

    using Map = std::vector<std::vector<std::pair<char, ObjectGuid>>>;
    using WorldCoordinates = std::unordered_map<std::pair<int32_t, int32_t>, STargetPosition>;
    using PseudoCoordinates = std::unordered_map<Position, std::pair<int32_t, int32_t>>;
    using Path = std::vector<std::pair<int32_t, int32_t>>;

    struct SMapInfo {
        int32_t width = 0;
        int32_t height = 0;
        float angle = 0;
        float delta_x = 0;
        float delta_y = 0;
        float square_width = 0;
        float square_height = 0;
    };

    std::string sprintf(const char* fmt, ...);

    class MapManager
    {
        /*friend class EventController;*/

        public:
            MapManager()  = delete;

            MapManager(std::string&& textMap);
            ~MapManager() = default;
            MapManager(const MapManager&) = default;
            MapManager& operator=(const MapManager&) = default;
            MapManager(MapManager&&) = default;
            MapManager& operator=(MapManager&&) = default;

            bool parse();
            void calculateArea(const SSquare& square);
            int32_t getWidth() const;
            int32_t getHeight() const;
            const Map& getGrid() const;
            char getGridCell(int32_t i, int32_t j) const;
            void placeCreatureOnGrid(int32_t i, int32_t j, ObjectGuid guid);
            void setCoordinateInfo(std::pair<int32_t, int32_t>& ij, STargetPosition&& pos);
            STargetPosition getCoordinateInfo(const std::pair<int32_t, int32_t>& ij);
            void setMapInfo(SMapInfo&& mapInfo);
            std::pair<int32_t, int32_t> toPseudoCellTransform(Position& worldPosition);
            std::pair<int32_t, int32_t> toPseudoCellTransform(const Position& worldPosition);
            const Position& getTeleportTarget(ETeleportTarget target);
            const SMapInfo& getMapInfo() const;

            bool isAccessible(int32_t i, int32_t j) const;
            bool isAccessible(char point) const;
            bool isValidRange(int32_t i, int32_t j) const;
            /* generates a random point until it hits an available one. */
            std::pair<int32_t, int32_t> getRandomNearPoint(int32_t centerI, int32_t centerJ) const;
            /* calculates all available points and selects a random one. I don't know which solution is better */
            std::pair<int32_t, int32_t> getRandomNearPointEx(int32_t centerI, int32_t centerJ) const;
            std::pair<int32_t, int32_t> getNearestAvailablePoint(const std::pair<int32_t, int32_t>& target);
            std::vector<Position> toWaypointPathTransform(const Path& path);

        private:

            struct IntPairHash {
                size_t operator()(const std::pair<int32_t, int32_t>& p) const {
                    size_t h1 = std::hash<int32_t>{}(p.first);
                    size_t h2 = std::hash<int32_t>{}(p.second);
                    return h1 ^ (h2 << 1);
                }
            };

            std::string _rawMap;
            Map _grid;
            WorldCoordinates _worldCoordinates;
//            PseudoCoordinates _pseudoCoordinates;
            SMapInfo _mapInfo;

            Position _leftTeleportTarget;
            Position _rightTeleportTarget;

/*            std::unordered_map<std::pair<int32_t, int32_t>, WaypointNode, IntPairHash> _waypointsCache;*/
            std::unordered_map<std::pair<int32_t, int32_t>, Position, IntPairHash> _waypointsCache;
            /*int32_t _lastWaypointId = 10000000;*/
    };

    class ObjectFactory {

            struct SObjectTemplate {
                uint32 entry = 0;
                EObjectType objectType = EObjectType::Unknown;
            };

        public:
            ObjectFactory()  = delete;

            ObjectFactory(Creature& owner);
            ~ObjectFactory() = default;
            ObjectFactory(const ObjectFactory&) = default;
            ObjectFactory& operator=(const ObjectFactory&) = default;
            ObjectFactory(ObjectFactory&&) = default;
            ObjectFactory& operator=(ObjectFactory&&) = default;

            void defineTemplates();
            GameObject* summonGameObject(const char literal, const STargetPosition& positionInfo);
            Creature* summonCreature(const char literal, const STargetPosition& positionInfo);

        private:
            Creature& _owner;
            std::map<char, SObjectTemplate> _objectTemplate;


    };

    class PathFinder {
        public:
            PathFinder()  = delete;

            PathFinder(const Map& grid);
            ~PathFinder() = default;


            Path findPath(std::pair<int32_t, int32_t> start, std::pair<int32_t, int32_t> end, int32_t depth);
            bool isAdjacent(const std::pair<int32_t, int32_t>& current, const std::pair<int32_t, int32_t>& target);

        private:
            struct Node {
                Node(int32_t x, int32_t y, int32_t g, int32_t h, Node* parent = nullptr)
                    : x(x), y(y), g(g), h(h), parent(parent) {}
                Node() = default;

                int32_t f() const {
                    return g + h;
                }

                int32_t x = 0, y = 0;
                int32_t g = 2048, h = 2048;
                Node* parent = nullptr;
            };
            struct CompareNode {
                bool operator()(const std::unique_ptr<Node>& a, const std::unique_ptr<Node>& b) const {
                    return a->f() > b->f();
                }
            };

            bool isValid(int32_t x, int32_t y, int32_t rows, int32_t cols) {
                return x >= 0 && x < rows && y >= 0 && y < cols && _grid[x][y].first != ctWALL_CELL;
            }

            const Map& _grid;
            const int32_t _width;
            const int32_t _height;
    };

    class EventController {
    public:
        EventController();
        ~EventController() = default;
        EventController(const EventController&) = default;
        EventController& operator=(const EventController&) = default;
        EventController(EventController&&) = default;
        EventController& operator=(EventController&&) = default;

        bool isRuning() const;
        void prepareEvent(Creature& owner);
        void defineTemplates();
        /*void calculateArea(const SSquare& square);*/

        void summonGameObjects();
        void summonCreatures();
        void removeCreature(ObjectGuid guid);
        bool spawnFruitIfNeeded();

        void reset();
        bool isEnd() const;
        void run(Player* player);

        void updatePlayerStatus(EventMap& events, ObjectGuid guid);
        void getPlayerPseudoPosition(int32_t& i, int32_t& j);

        int32_t lifeCheck(Player* player, EventMap& events);
        void handleFruit(ObjectGuid const& guid);
        void movePlayerToStartPosition(Player* player, EventMap& events);

        void pingGhosts(EPacmManHandlerAction action);
        uint64 getGhostPosition(EPacmManHandlerAction action) const;

        //DEBUG
        std::string getPlayerPosition();

    private:
        std::unique_ptr<MapManager> _mapManager = nullptr;
        std::unique_ptr<PathFinder> _pathFinder = nullptr;
        std::unique_ptr<ObjectFactory> _objectFactory = nullptr;
        std::unordered_map<ObjectGuid, GameObject*> _summonedObjects;
        std::unordered_map<ObjectGuid, Creature*> _summonedCreatures;

        Position _currentPlayerPosition;
        Position _playerStartPosition;

        /* delay UpdateAI case*/
        std::pair<int32_t, int32_t> _previousPlayerPseudoPosition;

        Creature* _blinky = nullptr;
        Creature* _inky = nullptr;
        Creature* _clyde = nullptr;

        std::vector<Creature*> _lifeCounter;
        std::vector<std::pair<int32_t, int32_t>> _fruitPositions;
        std::stack<std::pair<int32_t, int32_t>> _fruitsHallOfFame;
        int32_t _points = 0;

        /* for fruit summon calculations */
        int32_t _totalPoints = 0;
        int32_t _summonedFruits = 0;


        ::Map* _currentMap; // for find exist objects
        bool _running;
    };

} // namespace PacMan
