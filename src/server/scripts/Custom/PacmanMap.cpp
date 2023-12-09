#include "PacmanMap.h"
#include "G3D/g3dmath.h"
#include "GameObjectData.h"
#include "GameObject.h"
#include "Object.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "WaypointDefines.h"
#include <random>

namespace PacMan {

    EventController::EventController() : _running(false)
    {
    }

    bool EventController::isRuning() const
    {
        return _running;
    }

    void EventController::prepareEvent(Creature& owner)
    {
        if (_running) {
            return;
        }

        /* the source of the map can be anything, in this case just constexpr variable */
        std::string rawMap(TXT_MAP);
        _mapManager = std::make_unique<MapManager>(std::move(rawMap));
        _objectFactory = std::make_unique<ObjectFactory>(owner);


        /* there is no map validation */
        if (!_mapManager->parse()) {
            throw std::runtime_error("Map parsing failed");
        }

        _pathFinder = std::make_unique<PathFinder>(_mapManager->getGrid());

        _currentMap = owner.GetMap();

        defineTemplates(/*there may be a structure that matches the map cells and templates.*/);
        _mapManager->calculateArea(POS_MAP);
    }

    void EventController::defineTemplates()
    {
        /* there may be some logic for binding object templates */
        _objectFactory->defineTemplates();
    }

    void EventController::summonGameObjects()
    {
        if (_running) {
            return;
        }

        if (!_mapManager || _mapManager->getGrid().empty() || !_objectFactory) {
            throw std::runtime_error("MapManager or ObjectFactory is not initialized");
        }

        int32_t width = _mapManager->getWidth();
        int32_t height = _mapManager->getHeight();

        for (int32_t i = 0; i < _mapManager->getHeight(); ++i) {
            for (int32_t j = 0; j < _mapManager->getWidth(); ++j) {
                STargetPosition pos = _mapManager->getCoordinateInfo({ i, j });
                if (pos.position == Position{0, 0, 0, 0}) {
                    continue;
                }
                char literal = _mapManager->getGrid()[i][j].first;
                GameObject* summoned = _objectFactory->summonGameObject(literal, pos);
                if (summoned) {
                    _summonedObjects.emplace(summoned->GetGUID(), summoned);
                }
                /* else do nothing it is assumed that the cell should be empty. Otherwise, you can add error handling.*/
            }
        }
    }

    void EventController::summonCreatures()
    {
        if (_running) {
            return;
        }

        if (!_mapManager || _mapManager->getGrid().empty() || !_objectFactory) {
            throw std::runtime_error("MapManager or ObjectFactory is not initialized");
        }

        int32_t width = _mapManager->getWidth();
        int32_t height = _mapManager->getHeight();

        int32_t lifeCounter = 0;
        int32_t fruitCounter = 0;

        for (int32_t i = 0; i < _mapManager->getHeight(); ++i) {
            for (int32_t j = 0; j < _mapManager->getWidth(); ++j) {
                char literal = _mapManager->getGrid()[i][j].first;
                if (literal == ctFRUIT_POSITION_CELL) {
                    _fruitPositions.push_back(std::make_pair(i, j));
                    continue;
                }
                if (literal == ctFRUIT_CELL) {
                    ++fruitCounter;
                    _fruitsHallOfFame.push(std::make_pair(i, j));
                    continue;
                }
                if (literal == ctPLAYER_START_CELL) {
                    _playerStartPosition = _mapManager->getCoordinateInfo({ i, j }).position;
                    _playerStartPosition.SetOrientation(_playerStartPosition.GetOrientation() - M_PI_2);
                    continue;
                }

                STargetPosition pos = _mapManager->getCoordinateInfo({ i, j });
                if (pos.position == Position{ 0, 0, 0, 0 }) {
                    continue;
                }

                Creature* summoned = _objectFactory->summonCreature(literal, pos);
                if (summoned) {
                    ObjectGuid guid = summoned->GetGUID();
                    _summonedCreatures.emplace(guid, summoned);
                    switch (literal) {
                        case ctBLINKY_START_CELL: {
                            _blinky = summoned;
                            break;
                        }
                        case ctINKY_START_CELL: {
                            _inky = summoned;
                            break;
                        }
                        case ctCLYDE_START_CELL: {
                            _clyde = summoned;
                            break;
                        }
                        case ctPLAYER_LIFE_CELL: {
                            _lifeCounter.emplace_back(summoned);
                            ++lifeCounter;
                            break;
                        }
                        case ctBONUS_POINT_CELL:
                        case ctREGULAR_POINT_CELL: {
                            ++_points;
                            [[fallthrough]];
                        }
                        default: {
                            _mapManager->placeCreatureOnGrid(i, j, guid);
                            break;
                        }
                    }
                }
                /* else do nothing it is assumed that the cell should be empty. Otherwise, you can add error handling.*/
            }
        }

        if (lifeCounter != PACMAN_LIFE_COUNTER || fruitCounter != PACMAN_FRUIT_COUNTER) {
            throw std::logic_error("Invalid map configuration");
        }

        _totalPoints = _points;
    }

    /* in case if i should not manage object life */
    void EventController::removeCreature(ObjectGuid guid)
    {
        auto it = _summonedCreatures.find(guid);
        if (it != _summonedCreatures.end()) {
                _summonedCreatures.erase(it);
        }
    }

    bool EventController::spawnFruitIfNeeded()
    {
        --_points;

        bool needToSpawn = (_points % (_totalPoints / (PACMAN_FRUIT_COUNTER + 1)) == 0) && _points < _totalPoints - 2; // _points < _totalPoints - 2 skip first
        if (_summonedFruits < PACMAN_FRUIT_COUNTER && needToSpawn) {
            srand(static_cast<unsigned int>(time(nullptr)));
            int32_t random = rand() % _fruitPositions.size();
            auto& fruitPosition = _fruitPositions[random];
            STargetPosition pos = _mapManager->getCoordinateInfo(fruitPosition);
            Creature* summoned = _objectFactory->summonCreature(ctFRUIT_POSITION_CELL, pos);

            if (summoned) {
                _summonedCreatures.emplace(summoned->GetGUID(), summoned);
                _mapManager->placeCreatureOnGrid(fruitPosition.first, fruitPosition.second, summoned->GetGUID());
                ++_summonedFruits;
                return true;
            }
        }

        return false;
    }

    void EventController::reset()
    {
        _mapManager.release();
        _objectFactory.release();
        _pathFinder.release();

        for (auto& object : _summonedObjects) {
            /*upd: need to check existing creature because access violation if object was removed in-game (gm commands eg)*/
            GameObject* actualPtr = _currentMap->GetGameObject(object.first);
            if (actualPtr) {
                actualPtr->RemoveFromWorld();
            }
        }
        _summonedObjects.clear();
        for (auto& creature : _summonedCreatures) {
            Creature* actualPtr = _currentMap->GetCreature(creature.first);
            if (actualPtr) {
                actualPtr->RemoveFromWorld();
            }
        }
        _summonedCreatures.clear();
        _lifeCounter.clear();
        _fruitPositions.clear();
        std::stack<std::pair<int32_t, int32_t>> swap;
        _fruitsHallOfFame.swap(swap);

        _summonedFruits = 0;
        _points = 0;
        _totalPoints = 0;

        _blinky = nullptr;
        _inky = nullptr;
        _clyde = nullptr;

        _running = false;
    }

    bool EventController::isEnd() const
    {
        return _points == 0;
    }

    void EventController::run(Player* player)
    {
        if (_running || !player) {
            return;
        }

        for (auto& model : _lifeCounter) {
            player->CastSpell(model, SPELL_CLONE_PLAYER, true);
        }

        _running = true;
    }

    void EventController::updatePlayerStatus(EventMap& events, ObjectGuid guid)
    {
        if (!_running) {
            return;
        }

        Player* player = _currentMap->GetPlayer(guid);
        if (!player || player->GetMapId() != _currentMap->GetId() || !player->IsAlive() || player->isAFK() || player->IsFlying()) {
            events.ScheduleEvent(STOP_FAIL, 1ms);
            return;
        }

        _currentPlayerPosition = player->GetPosition();

        std::pair<int32_t, int32_t> ij = _mapManager->toPseudoCellTransform(_currentPlayerPosition);
        if (ij.first >= _mapManager->getHeight() || ij.first < 0 ||
            ij.second >= _mapManager->getWidth() || ij.second < 0) {
            /* normalize position */
            ij.first = (ij.first < 0) ? 0 :
                       (ij.first > _mapManager->getHeight()) ? _mapManager->getHeight() - 1 : ij.first;
            ij.second = (ij.second < 0) ? 0 :
                        (ij.second > _mapManager->getWidth()) ? _mapManager->getWidth() - 1 : ij.second;

            Path restoredPlayerPath = _pathFinder->findPath(_previousPlayerPseudoPosition, ij, 4);

            /* jump over teleport case */
            if (std::find_if(restoredPlayerPath.begin(), restoredPlayerPath.end(), [this, &ij](const std::pair<int32_t, int32_t>& ijIt) {
                    char mapLiteral = _mapManager->getGrid()[ijIt.first][ijIt.second].first;
                    if (mapLiteral == ctRIGHT_TP_POINT || mapLiteral == ctLEFT_TP_POINT) {
                        /* force teleport position as current position */
                        ij = ijIt;
                        return true;
                    }
                    return false;
            }) == restoredPlayerPath.end()) {
                events.ScheduleEvent(STOP_FAIL, 1ms);
                return;
            }
        }

        char mapLiteral = _mapManager->getGrid()[ij.first][ij.second].first;
        if (mapLiteral == ctRIGHT_TP_POINT) {
            Position pos = _mapManager->getTeleportTarget(ETeleportTarget::Right);
            /* there is no solution for pathfinder, but you can add a method that receives a literal by index,
               or save the teleport points as a field and set it as the previous player position target teleport index */
            _previousPlayerPseudoPosition = ij;
            player->NearTeleportTo(pos);
            /* there should be no interactive objects near the teleport except pursuing monsters */
            return;
        }
        else if (mapLiteral == ctLEFT_TP_POINT) {
            Position pos = _mapManager->getTeleportTarget(ETeleportTarget::Left);
            _previousPlayerPseudoPosition = ij;
            player->NearTeleportTo(pos);
            return;
        }
        else if (mapLiteral == ctPLAYER_START_CELL) {
            if (_previousPlayerPseudoPosition == std::pair<int32_t, int32_t>{0, 0}) {
                _previousPlayerPseudoPosition = ij;
            }
            return;
        }
        else if (mapLiteral == ctWALL_CELL) {
            movePlayerToStartPosition(player, events);
            return;
        }

        /* Workaround - UpdateAI works too slowly to process disappearing monsters. Therefore, we are trying to restore the player's path with dept 4
           It is possible that due to server delays, the solution will not work as expected..*/
        Path restoredPlayerPath = _pathFinder->findPath(_previousPlayerPseudoPosition, ij, 4);
        restoredPlayerPath.emplace_back(ij);

        for (auto& ijIt : restoredPlayerPath) {
            /* check idle creature on this pseudo-cell */
            ObjectGuid creatureGuid = _mapManager->getGrid()[ijIt.first][ijIt.second].second;

            if (creatureGuid != ObjectGuid::Empty) {
                Creature* creature = _currentMap->GetCreature(creatureGuid);
                if (creature) {
                    creature->GetAI()->DoAction(CREATURE_TOUCHED_BY_PLAYER);
                    if (mapLiteral == ctREGULAR_POINT_CELL || mapLiteral == ctBONUS_POINT_CELL || mapLiteral == ctFRUIT_POSITION_CELL) {
                        _mapManager->placeCreatureOnGrid(ijIt.first, ijIt.second, ObjectGuid::Empty);
                    }
                }
            }
        }

        _previousPlayerPseudoPosition = ij;
    }

    void EventController::getPlayerPseudoPosition(int32_t& i, int32_t& j)
    {
        std::pair<int32_t, int32_t> ij = _mapManager->toPseudoCellTransform(_currentPlayerPosition);
        i = ij.first;
        j = ij.second;
    }

    int32_t EventController::lifeCheck(Player* player, EventMap& events)
    {
        int32_t lifeCounter = PACMAN_LIFE_COUNTER;
        for (auto it = _lifeCounter.rbegin(); it != _lifeCounter.rend(); ++it) {
            if (*it != nullptr) {
                (*it)->KillSelf();
                (*it)->DespawnOrUnsummon(1000ms);
                *it = nullptr;
                movePlayerToStartPosition(player, events);
                return lifeCounter;
            }
            --lifeCounter;
        }

        return lifeCounter;
    }

    void EventController::handleFruit(ObjectGuid const& guid)
    {
        if (_fruitsHallOfFame.empty()) {
            /*assert*/
            return;
        }

        std::pair<int32_t, int32_t> pos = _fruitsHallOfFame.top();
        _fruitsHallOfFame.pop();

        Creature* fruit = _currentMap->GetCreature(guid);
        if (fruit) {
            std::pair<int32_t, int32_t> savedPos = _mapManager->toPseudoCellTransform(fruit->GetPosition());
            fruit->NearTeleportTo(_mapManager->getCoordinateInfo(pos).position);
            fruit->CastSpell(nullptr, SPELL_NPC_TELEPORT_VISUAL);
            _mapManager->placeCreatureOnGrid(savedPos.first, savedPos.second, guid);
        }
    }

    void EventController::movePlayerToStartPosition(Player* player, EventMap& events)
    {
        /* dead and start event by same player case */
        _previousPlayerPseudoPosition = _mapManager->toPseudoCellTransform(_playerStartPosition);

        player->NearTeleportTo(_playerStartPosition, true);
        player->CastSpell(nullptr, SPELL_STUN_VISUAL);
        player->CastSpell(nullptr, SPELL_SELF_STUN);
        events.ScheduleEvent(REMOVE_PLAYER_STUN, PLAYER_STUN_DURATION);
    }

    void EventController::pingGhosts(EPacmManHandlerAction action)
    {
        if (!_running) {
            return;
        }

        switch (action) {
            case EPacmManHandlerAction::BLINKY_LOSE:
            case EPacmManHandlerAction::BLINKY_WIN: {
                if (_inky && _clyde) {
                    _inky->GetAI()->DoAction(action);
                    _clyde->GetAI()->DoAction(action);
                }
                break;
            }
            case EPacmManHandlerAction::INKY_LOSE:
            case EPacmManHandlerAction::INKY_WIN: {
                if (_blinky && _clyde) {
                    _clyde->GetAI()->DoAction(action);
                    _blinky->GetAI()->DoAction(action);
                }
                break;
            }
            case EPacmManHandlerAction::CLYDE_LOSE:
            case EPacmManHandlerAction::CLYDE_WIN: {
                if (_inky && _blinky) {
                    _inky->GetAI()->DoAction(action);
                    _blinky->GetAI()->DoAction(action);
                }
                break;
            }
            case EPacmManHandlerAction::STOP_GHOSTS:
            case EPacmManHandlerAction::BONUS_POINT_DESPAWN_HA: {
                if (_inky) {
                    _inky->GetAI()->DoAction(action);
                }
                if (_blinky) {
                    _blinky->GetAI()->DoAction(action);
                }
                if (_clyde) {
                    _clyde->GetAI()->DoAction(action);
                }
                break;
            }
            default: {
                break;
            }
        }
    }

    uint64 EventController::getGhostPosition(EPacmManHandlerAction action) const
    {
        switch (action)
        {
            case BLINKY: {
                if (!_blinky) {
                    return 0;
                }
                Position pos = _blinky->GetPosition();
                std::pair<int32_t, int32_t> ij = _mapManager->toPseudoCellTransform(pos);

                /* check endian */
                uint16_t num = 0x01;
                uint8_t* ptr = reinterpret_cast<uint8_t*>(&num);
                bool isBigEndian = ptr[0] == 0x00;

                uint64 packed = static_cast<uint64_t>(ij.first) << 32 | ij.second;
                if (isBigEndian) {
                    /* reverse byte order */
                    packed = (packed >> 32) | (packed << 32);
                }
                return packed;
            }
            case INKY:
            case CLYDE:
            default:
                return 0;
        }

    }

    std::string EventController::getPlayerPosition()
    {
        std::pair<int32_t, int32_t> ij = _mapManager->toPseudoCellTransform(_currentPlayerPosition);

        auto r = _pathFinder->findPath(_previousPlayerPseudoPosition, ij, 15);
        std::string path = "[" + std::to_string(ij.first) + ":" + std::to_string(ij.second) + "]: ";
        for (auto& ijIt : r) {
            path += "(" + std::to_string(ijIt.first) + ":" + std::to_string(ijIt.second) + ") ";
        }
        return path;
    }

    MapManager::MapManager(std::string&& textMap) : _rawMap(textMap) {}

    bool MapManager::parse()
    {
        std::vector<std::string> rows;
        size_t startPos = 0;
        size_t foundPos = _rawMap.find("\n", startPos);
        while (foundPos != std::string::npos) {
            rows.emplace_back(_rawMap.substr(startPos, foundPos - startPos));
            startPos = foundPos + 1;
            foundPos = _rawMap.find("\n", startPos);
        }
        if (startPos < _rawMap.size()) {
            std::string lastRow = _rawMap.substr(startPos);
            rows.push_back(lastRow);
        }

        for (const std::string& row : rows) {
            std::vector<std::pair<char, ObjectGuid>> map_row;
            std::for_each(row.begin(), row.end(), [&map_row](char c) { map_row.push_back(std::make_pair(c, ObjectGuid::Empty)); });
            _grid.push_back(map_row);
        }

        _mapInfo.width = _grid[0].size();
        _mapInfo.height = _grid.size();

        return true;
    }

    void MapManager::calculateArea(const SSquare& square)
    {
        /* area size */
        float width = square.right_top.x - square.left_top.x;
        float height = square.left_bottom.y - square.left_top.y;
        float depth_top = square.left_top.z - square.right_top.z;
        float depth_bottom = square.left_bottom.z - square.right_bottom.z;

        /* one square size */
        float square_width = width / _mapInfo.width;
        float square_height = height / _mapInfo.height;
        _mapInfo.square_height = square_height; _mapInfo.square_width = square_width;

        /* rectangle rotation angle */
        float angle = static_cast<float>(G3D::aTan2(square.left_top.y - square.right_top.y, square.left_top.x - square.right_top.x));
        _mapInfo.angle = angle;

        float delta_x = square.left_top.x;
        float delta_y = square.left_top.y;
        _mapInfo.delta_x = delta_x; _mapInfo.delta_y = delta_y;

        for (int32_t i = 0; i < _mapInfo.height; ++i) {
            for (int32_t j = 0; j < _mapInfo.width; ++j) {

                /* find the center of the current square */
                float center_x = j * square_width + square_width / 2;
                float center_y = i * square_height + square_height / 2;

                /* APPROXIMATE Interpolation factor to determine the Z value in a given square */
                float interpolation_coefficient_x = center_x / width;
                float z_top = square.left_top.z + (square.right_top.z - square.left_top.z) * interpolation_coefficient_x;
                float z_bottom = square.left_bottom.z + (square.right_bottom.z - square.left_bottom.z) * interpolation_coefficient_x;

                /* Correcting coordinates taking into account displacement and rotation */
                float rotated_x = center_x * cos(angle) - center_y * sin(angle);
                float rotated_y = center_x * sin(angle) + center_y * cos(angle);

                float final_x = delta_x - rotated_x;
                float final_y = delta_y - rotated_y;
                float interpolation_coefficient_y = center_y / height;
                float final_z = z_top - (z_top - z_bottom) * interpolation_coefficient_y;

                std::pair<int32_t, int32_t> ij = { i, j };
                QuaternionData quanterion = QuaternionData::fromEulerAnglesZYX(angle, 0, 0);;
                Position position = { final_x, final_y, final_z, angle };
                STargetPosition pos = { quanterion, position };
                setCoordinateInfo(ij, std::move(pos));
            }
        }
    }

    int32_t MapManager::getWidth() const
    {
        return _mapInfo.width;
    }

    int32_t MapManager::getHeight() const
    {
        return _mapInfo.height;
    }

    const Map& MapManager::getGrid() const
    {
        return _grid;
    }

    char MapManager::getGridCell(int32_t i, int32_t j) const
    {
        if (i < 0 || i >= _mapInfo.height || j < 0 || j >= _mapInfo.width) {
            return '0';
        }
        return _grid[i][j].first;
    }

    void MapManager::placeCreatureOnGrid(int32_t i, int32_t j, ObjectGuid guid)
    {
        if (i < 0 || i >= _mapInfo.height || j < 0 || j >= _mapInfo.width) {
            return;
        }
        _grid[i][j].second = guid;
    }

    void MapManager::setCoordinateInfo(std::pair<int32_t, int32_t>& ij, STargetPosition&& pos)
    {
        _worldCoordinates[ij] = pos;
        if (_grid[ij.first][ij.second].first == ctLEFT_TP_TARGET) {
            _leftTeleportTarget = pos.position;
            _leftTeleportTarget.SetOrientation(_mapInfo.angle);
        }
        else if (_grid[ij.first][ij.second].first == ctRIGHT_TP_TARGET) {
            _rightTeleportTarget = pos.position;
            _rightTeleportTarget.SetOrientation(_mapInfo.angle + M_PI);
        }
    }

    PacMan::STargetPosition MapManager::getCoordinateInfo(const std::pair<int32_t, int32_t>& ij)
    {
        auto it = _worldCoordinates.find(ij);
        if (it == _worldCoordinates.end()) {
            return STargetPosition{};
        }
        return it->second;
    }

    void MapManager::setMapInfo(SMapInfo&& mapInfo)
    {
        _mapInfo = mapInfo;
    }

    std::pair<int32_t, int32_t> MapManager::toPseudoCellTransform(Position& worldPosition)
    {
        int32_t i, j;
        j = static_cast<int32_t>(((_mapInfo.delta_x - worldPosition.GetPositionX()) * cos(_mapInfo.angle) +
                                 (_mapInfo.delta_y - worldPosition.GetPositionY()) * sin(_mapInfo.angle)) / _mapInfo.square_width);
        i = static_cast<int32_t>((-(_mapInfo.delta_x - worldPosition.GetPositionX()) * sin(_mapInfo.angle) +
                                 (_mapInfo.delta_y - worldPosition.GetPositionY()) * cos(_mapInfo.angle)) / _mapInfo.square_height);
        return { i, j };
    }

    std::pair<int32_t, int32_t> MapManager::toPseudoCellTransform(const Position& worldPosition)
    {
        int32_t i, j;
        j = static_cast<int32_t>(((_mapInfo.delta_x - worldPosition.GetPositionX()) * cos(_mapInfo.angle) +
            (_mapInfo.delta_y - worldPosition.GetPositionY()) * sin(_mapInfo.angle)) / _mapInfo.square_width);
        i = static_cast<int32_t>((-(_mapInfo.delta_x - worldPosition.GetPositionX()) * sin(_mapInfo.angle) +
            (_mapInfo.delta_y - worldPosition.GetPositionY()) * cos(_mapInfo.angle)) / _mapInfo.square_height);
        return { i, j };
    }

    const Position& MapManager::getTeleportTarget(ETeleportTarget target)
    {
        switch (target)
        {
        case PacMan::ETeleportTarget::Left:
            return _leftTeleportTarget;
        case PacMan::ETeleportTarget::Right:
            return _rightTeleportTarget;
        default:
            /* Should not happen. Just in case, so as not to create an extra default value and not return empty */
            return _leftTeleportTarget;
        }
    }

    const PacMan::SMapInfo& MapManager::getMapInfo() const
    {
        return _mapInfo;
    }

    bool MapManager::isAccessible(int32_t i, int32_t j) const
    {
        return isAccessible(_grid[i][j].first);
    }

    bool MapManager::isAccessible(char point) const
    {
        return (point == ctEMPTY_CELL || point == ctREGULAR_POINT_CELL || point == ctBONUS_POINT_CELL ||
                point == ctPLAYER_START_CELL || point == ctBLINKY_START_CELL || point == ctLEFT_TP_TARGET ||
                point == ctRIGHT_TP_TARGET || point == ctFRUIT_POSITION_CELL);
    }

    bool MapManager::isValidRange(int32_t i, int32_t j) const
    {
        return (i >= 0 && i < _mapInfo.height && j >= 0 && j < _mapInfo.width);
    }

    std::pair<int32_t, int32_t> MapManager::getRandomNearPoint(int32_t centerI, int32_t centerJ) const
    {
        std::random_device rd;
        std::mt19937 gen(rd());

        int32_t startJ = std::max(0, centerJ - _mapInfo.width / 2);
        int32_t endJ = std::min(_mapInfo.width - 1, centerJ + _mapInfo.width / 2);
        int32_t startI = std::max(0, centerI - _mapInfo.height / 2);
        int32_t endI = std::min(_mapInfo.height - 1, centerI + _mapInfo.height / 2);

        std::uniform_int_distribution<int32_t> distribJ(startJ, endJ);
        std::uniform_int_distribution<int32_t> distribI(startI, endI);

        int32_t randomJ = distribJ(gen);
        int32_t randomI = distribI(gen);

        while (!isAccessible(_grid[randomI][randomJ].first)) {
            randomJ = distribJ(gen);
            randomI = distribI(gen);
        }

        return std::make_pair(randomI, randomJ);
    }

    std::pair<int32_t, int32_t> MapManager::getRandomNearPointEx(int32_t centerI, int32_t centerJ) const
    {
        std::vector<std::pair<int32_t, int32_t>> accessiblePoints;

        int32_t startJ = std::max(0, centerJ - _mapInfo.width / 2);
        int32_t endJ = std::min(_mapInfo.width - 1, centerJ + _mapInfo.width / 2);
        int32_t startI = std::max(0, centerI - _mapInfo.height / 2);
        int32_t endI = std::min(_mapInfo.height - 1, centerI + _mapInfo.height / 2);

        for (int32_t i = startI; i <= endI; ++i) {
            for (int32_t j = startJ; j <= endJ; ++j) {
                if (isAccessible(_grid[i][j].first)) {
                    accessiblePoints.push_back(std::make_pair(i, j));
                }
            }
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int32_t> distrib(0, accessiblePoints.size() - 1);
        return accessiblePoints[distrib(gen)];
    }

    std::pair<int32_t, int32_t> MapManager::getNearestAvailablePoint(const std::pair<int32_t, int32_t>& target)
    {
        std::vector<std::vector<bool>> visited(_mapInfo.height, std::vector<bool>(_mapInfo.width, false));

        std::queue<std::pair<int32_t, int32_t>> q;
        q.push(target);
        visited[target.first][target.second] = true;

        int dj[] = { 0, 0, 1, -1 };
        int di[] = { 1, -1, 0, 0 };

        while (!q.empty()) {
            std::pair<int32_t, int32_t> current = q.front();
            q.pop();

            for (int k = 0; k < 4; ++k) {
                int newI = current.first + di[k];
                int newJ = current.second + dj[k];

                if (isValidRange(newI, newJ) && !visited[newI][newJ]) {
                    visited[newI][newJ] = true;

                    if (isAccessible(newI, newJ)) {
                        return { newI, newJ };
                    }

                    q.push({ newI, newJ });
                }
            }
        }

        return target;
    }

    std::vector<Position> MapManager::toWaypointPathTransform(const Path& path)
    {
        std::vector<Position> result;

        /*int32_t id = _lastWaypointId;*/
        for (const auto& point : path) {
            if (_waypointsCache[point].m_positionX == 0) {
                Position pos = getCoordinateInfo(point).position;
                /*WaypointNode newNode(id++, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());*/
                _waypointsCache[point] = std::move(pos);
            }

            result.push_back(_waypointsCache[point]);
        }

        /* i will no reuse WayPointPath i think */
        /*return WaypointPath(id, std::move(result));*/
        return result;
    }

    ObjectFactory::ObjectFactory(Creature& owner) : _owner(owner), _objectTemplate{} {}

    void ObjectFactory::defineTemplates()
    {
        _objectTemplate['X'] = { 188236, EObjectType::GameObject }; // box
        _objectTemplate['1'] = { 43501, EObjectType::Creature }; // regular point
        _objectTemplate['2'] = { 43505, EObjectType::Creature }; // bonus point
        _objectTemplate['B'] = { 43502, EObjectType::Creature }; // Blinky
        _objectTemplate['I'] = { 43503, EObjectType::Creature }; // Inky
        _objectTemplate['c'] = { 43504, EObjectType::Creature }; // Clyde
        _objectTemplate['P'] = { 43506, EObjectType::Creature }; // life counter
        _objectTemplate['3'] = { 43507, EObjectType::Creature }; // fruit
    }

    GameObject* ObjectFactory::summonGameObject(const char literal, const STargetPosition& positionInfo)
    {
        auto templateIt = _objectTemplate.find(literal);
        if (templateIt == _objectTemplate.end() || templateIt->second.objectType != EObjectType::GameObject) {
            return nullptr;
        }

        uint32 entry = templateIt->second.entry;
        GameObject* gameObject = _owner.SummonGameObject(entry, positionInfo.position, positionInfo.quanterion, Seconds(0));
        if (!gameObject) {
            return nullptr;
        }
        gameObject->SetOwnerGUID(_owner.GetGUID());
        return gameObject;
    }

    Creature* ObjectFactory::summonCreature(const char literal, const STargetPosition& positionInfo)
    {
        auto templateIt = _objectTemplate.find(literal);
        if (templateIt == _objectTemplate.end() || templateIt->second.objectType != EObjectType::Creature) {
            return nullptr;
        }

        uint32 entry = templateIt->second.entry;
        Position position = positionInfo.position;
        position.SetOrientation(position.GetOrientation() - M_PI_2);
        Creature* creature = _owner.SummonCreature(entry, position);
        if (!creature) {
            return nullptr;
        }
        creature->SetOwnerGUID(_owner.GetGUID());
        return creature;
    }

    PathFinder::PathFinder(const Map& grid) : _grid(grid), _width(grid[0].size()), _height(grid.size()) {}

    Path PathFinder::findPath(std::pair<int32_t, int32_t> start, std::pair<int32_t, int32_t> end, int32_t depth) {
        std::vector<std::pair<int32_t, int32_t>> directions = { {-1, 0}, {0, -1}, {1, 0}, {0, 1} };

        std::vector<std::vector<bool>> closedSet(_height, std::vector<bool>(_width, false));
        std::vector<std::vector<Node>> nodes(_height, std::vector<Node>(_width));

        Node* startNode = &nodes[start.first][start.second];
        *startNode = Node(start.first, start.second, 0, std::abs(end.first - start.first) + std::abs(end.second - start.second), nullptr);

        int32_t depthCounter = 0;

        while (depthCounter <= depth) {
            Node* currentNode = nullptr;
            int32_t minCost = INT_MAX;

            for (int32_t i = 0; i < _height; ++i) {
                for (int32_t j = 0; j < _width; ++j) {
                    if (!closedSet[i][j] && nodes[i][j].g + nodes[i][j].h < minCost) {
                        currentNode = &nodes[i][j];
                        minCost = currentNode->g + currentNode->h;
                    }
                }
            }

            if (currentNode == nullptr) break;

            int32_t x = currentNode->x;
            int32_t y = currentNode->y;

            if (x == end.first && y == end.second) {
                Path path;
                while (currentNode != nullptr) {
                    path.emplace_back(currentNode->x, currentNode->y);
                    currentNode = currentNode->parent;
                }
                std::reverse(path.begin(), path.end());
                return path;
            }

            closedSet[x][y] = true;

            for (const auto& dir : directions) {
                int32_t newX = x + dir.first;
                int32_t newY = y + dir.second;

                if (isValid(newX, newY, _height, _width) && !closedSet[newX][newY]) {
                    int32_t g = currentNode->g + 1;
                    int32_t h = std::abs(end.first - newX) + std::abs(end.second - newY);
                    if (g + h < nodes[newX][newY].g + nodes[newX][newY].h || nodes[newX][newY].g + nodes[newX][newY].h == 0) {
                        nodes[newX][newY] = Node(newX, newY, g, h, currentNode);
                    }
                }
            }

            depthCounter++;
        }

        int32_t minDistance = INT_MAX;
        Node* closestNode = nullptr;

        for (int32_t i = 0; i < _height; ++i) {
            for (int32_t j = 0; j < _width; ++j) {
                if (closedSet[i][j]) {
                    int32_t distance = std::abs(end.first - i) + std::abs(end.second - j);
                    if (distance < minDistance) {
                        minDistance = distance;
                        closestNode = &nodes[i][j];
                    }
                }
            }
        }

        Path path;
        while (closestNode != nullptr) {
            path.emplace_back(closestNode->x, closestNode->y);
            closestNode = closestNode->parent;
        }
        std::reverse(path.begin(), path.end());

        return path;
    }

    bool PathFinder::isAdjacent(const std::pair<int32_t, int32_t>& current, const std::pair<int32_t, int32_t>& target)
    {
        int dj = std::abs(current.second - target.second);
        int di = std::abs(current.first - target.first);

        return (dj <= 1 && di <= 1) || current == target;
    }

    std::string sprintf(const char* fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);

        const int bufferSize = 256;
        char buffer[bufferSize];

        vsnprintf(buffer, bufferSize, fmt, ap);
        va_end(ap);

        return std::string(buffer);
    }

} // namespace PacMan


