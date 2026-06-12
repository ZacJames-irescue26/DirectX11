local yaw = 0.0
local pitch = 0.0

local mouseSensitivity = 0.002
local stickLookSensitivity = 2.5
local moveSpeed = 6.0
local pitchLimit = 1.55

-- Gun settings
local gunRange = 1000.0
local gunDamage = 25.0
local fireCooldown = 1.0
local timeSinceLastShot = 0.0

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
function Vec3ToString(v)
    return "(" .. v.x .. ", " .. v.y .. ", " .. v.z .. ")\n"
end
function FireGun(entity)
    local camera = entity:GetChildByName("Camera")
    local audioPlayer = entity:GetChildByName("Audio")

    Logging.LogToConsol("Started fire")
    Audio.PlayEntityAudio(audioPlayer)
    if camera == nil then
        return
    end

    local camTransform = camera:GetTransform()

    if camTransform == nil then
        return
    end
    local origin = camera:GetWorldPosition()

    -- Since your controller stores yaw on the body and pitch on the camera,
    -- build the forward direction from yaw + pitch.
    local cosPitch = math.cos(pitch)
    local sinPitch = math.sin(pitch)

    local sinYaw = math.sin(yaw)
    local cosYaw = math.cos(yaw)

    local direction = Vec3.new(
        sinYaw * cosPitch,
        -sinPitch,
        cosYaw * cosPitch
    )
    local cameracomp = camera:GetCamera()
    direction = cameracomp:GetForward()
    -- Exclude the player entity so the ray does not hit yourself.
    local exclude = {
        entity:GetUUID()
    }

    local hit = Physics.Raycast(
        origin,
        direction,
        gunRange,
        exclude
    )
    Logging.LogToConsol(Vec3ToString(direction))
    if hit.hit then
        Logging.LogToConsol(hit.entity)
        Logging.LogToConsol("Distance: " .. hit.distance)
        Logging.LogToConsol(GetEntityByUUID(hit.entity):GetName())
        Entity.ApplyDamage(hit.entity, 10)
        -- Optional, if you expose these functions:
        -- Entity.ApplyDamage(hit.entity, gunDamage)
        -- Effects.SpawnImpact(hit.position, hit.normal)

        Debug.DrawLine(origin, hit.position, Vec3.new(1.0, 0.0, 0.0))
    else
        local endPos = Vec3.new(
            origin.x + direction.x * gunRange,
            origin.y + direction.y * gunRange,
            origin.z + direction.z * gunRange
        )

        Debug.DrawLine(origin, endPos, Vec3.new(1.0, 1.0, 1.0), 0.25)
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
    pitch = pitch + mouseDY * mouseSensitivity

    yaw = yaw - stickRX * stickLookSensitivity * dt
    pitch = pitch - stickRY * stickLookSensitivity * dt

    if pitch > pitchLimit then
        pitch = pitchLimit
    end

    if pitch < -pitchLimit then
        pitch = -pitchLimit
    end

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

    moveX = moveX - rightX * leftX
    moveZ = moveZ - rightZ * leftX

    moveX = moveX + forwardX * leftY
    moveZ = moveZ + forwardZ * leftY

    local len = math.sqrt(moveX * moveX + moveZ * moveZ)

    if len > 1.0 then
        moveX = moveX / len
        moveZ = moveZ / len
    end

    local velocity = Physics.GetLinearVelocity(entity)
    local currentYVelocity = velocity.y

    Physics.SetLinearVelocity(
        entity,
        Vec3.new(moveX * moveSpeed, currentYVelocity, moveZ * moveSpeed)
    )

    if Controller.IsButtonDown(GamepadButton.A) then
        Physics.AddImpulse(entity, Vec3.new(0.0, 200.0, 0.0))
    end

    -- Gun update
    timeSinceLastShot = timeSinceLastShot + dt

    local wantsToFire =
        Mouse.IsLeftMouseDown()
    if Controller.RightTrigger() > 0.2 then
        wantsToFire = true
    end
    if wantsToFire and timeSinceLastShot >= fireCooldown then
        timeSinceLastShot = 0.0
        FireGun(entity)
    end
    

end