---@class Vec3
---@field x number
---@field y number
---@field z number

---@class TransformComponent
---@field Position Vec3
---@field Rotation Vec3
---@field Scale Vec3

---@class Entity
Entity = {}

---@return string
function Entity:GetName() end

---@return TransformComponent|nil
function Entity:GetTransform() end

---@type Entity
entity = nil