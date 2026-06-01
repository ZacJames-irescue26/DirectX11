local yaw = 0.0
local pitch = 0.0

local mouseSensitivity = 0.002
local stickLookSensitivity = 2.5
local moveSpeed = 6.0
local pitchLimit = 1.55

function OnCreate(entity)
    local transform = entity:GetTransform()

    if transform ~= nil then
        yaw = transform.RotationEuler.y
    end

    local camera = entity:GetChildByName("Camera")

    if camera ~= nil then
        local camTransform = camera:GetTransform()

        if camTransform ~= nil then
            pitch = camTransform.RotationEuler.x
        end
    end
end

function OnUpdate(entity, dt)
    local transform = entity:GetTransform()

    if transform == nil then
        return
    end

    local camera = entity:GetChildByName("Camera")
    local camTransform = nil

    if camera ~= nil then
        camTransform = camera:GetTransform()
    end

    local mouseDX = Mouse.GetDeltaX()
    local mouseDY = Mouse.GetDeltaY()
    
    local stickRX = Controller.RightX()
    local stickRY = Controller.RightY()
    
    yaw = yaw + mouseDX * mouseSensitivity
    pitch = pitch - mouseDY * mouseSensitivity
    
    yaw = yaw + stickRX * stickLookSensitivity * dt
    pitch = pitch + stickRY * stickLookSensitivity * dt
    
    if pitch > pitchLimit then
        pitch = pitchLimit
    end
    
    if pitch < -pitchLimit then
        pitch = -pitchLimit
    end
    
    local yawQuat = MathEx.QuatFromEuler(Vec3.new(0.0, yaw, 0.0))
    Physics.SetRotationEuler(entity, Vec3.new(0.0, yaw, 0.0))
    
    if camTransform ~= nil then
        camTransform:SetRotationEuler(Vec3.new(pitch, 0.0, 0.0))
    end

    local forwardX = math.sin(yaw)
    local forwardZ = math.cos(yaw)

    local rightX = math.cos(yaw)
    local rightZ = -math.sin(yaw)

    local moveX = 0.0
    local moveZ = 0.0

    if Input.IsKeyDown("W") then
        moveX = moveX + forwardX
        moveZ = moveZ + forwardZ
    end

    if Input.IsKeyDown("S") then
        moveX = moveX - forwardX
        moveZ = moveZ - forwardZ
    end

    if Input.IsKeyDown("D") then
        moveX = moveX + rightX
        moveZ = moveZ + rightZ
    end

    if Input.IsKeyDown("A") then
        moveX = moveX - rightX
        moveZ = moveZ - rightZ
    end

    local leftX = Controller.LeftX()
    local leftY = Controller.LeftY()

    moveX = moveX + rightX * leftX
    moveZ = moveZ + rightZ * leftX

    moveX = moveX + forwardX * leftY
    moveZ = moveZ + forwardZ * leftY

    local len = math.sqrt(moveX * moveX + moveZ * moveZ)

    if len > 1.0 then
        moveX = moveX / len
        moveZ = moveZ / len
    end

   local velocity = Physics.GetLinearVelocity(entity)
    local currentYVelocity = velocity.y

    Physics.SetLinearVelocity(entity, Vec3.new(moveX * moveSpeed, currentYVelocity, moveZ * moveSpeed))

    if Controller.IsButtonDown(GamepadButton.A) then
        Physics.AddImpulse(entity, Vec3.new(0.0,200,0.0))
    end
end