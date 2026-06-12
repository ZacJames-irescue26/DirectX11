
function OnCreate(entity)

	Entity.AddPatrolAgent(entity, 10, Vec3.new(0.0,0.0,0.0),3.0)
	Entity.AddHealth(entity, 100)
    Animation.SetLooping(entity, true)

end


function OnUpdate(entity, dt)

    local authoredWalkSpeed = 1.8
    local mesh = entity:GetChildByName("Enemy Mesh")
    local velocity = Physics.GetLinearVelocity(entity)

    local horizontalSpeed = math.sqrt(
        velocity.x * velocity.x +
        velocity.z * velocity.z
    )

    if horizontalSpeed < 0.05 then
        Animation.Play(mesh, "Idle")
        Animation.SetSpeed(mesh, 1.0)
    else
        local playbackSpeed =
            horizontalSpeed / authoredWalkSpeed

        playbackSpeed = math.max(
            0.25,
            math.min(playbackSpeed, 2.5)
        )

        Animation.Play(mesh, "RunForward")
        Animation.SetSpeed(mesh, playbackSpeed)
    end

	local health = Entity.GetHealth(entity)
	if health.health <= 0.0 then
		entity:DestroyEntity()
	end

end