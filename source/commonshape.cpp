#include "commonshape.hpp"

std::vector<commonshapeObject*> commonshapeList;

commonshapeObject::commonshapeObject(){
	commonshapeList.push_back(this);
}

commonshapeObject::~commonshapeObject(){
}


void commonshapeObject::addVertex(vertex newvertex){
	objectData.push_back(newvertex);
}

void commonshapeObject::registerToSystem(){
		registervertex(&objectData, &indexBufferArray);

		glGenBuffers(1, &indexBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferArray.size() * sizeof(GLuint), &indexBufferArray[0], GL_STATIC_DRAW);


		glGenBuffers(1, &instanceMatrixBuffer);
}

shapePointerObject* commonshapeObject::create(){
	return new shapePointerObject();
}

shapePointerObject* commonshapeObject::create(glm::vec3 position, glm::vec3 size, glm::quat quat){
	return new shapePointerObject(this, false, NULL, position, size, quat);
}

shapePointerObject* commonshapeObject::create(glm::vec3 position, glm::vec3 size, glm::quat quat, btScalar mass, btDiscreteDynamicsWorld *dynamicsWorld){

		std::vector<btVector3> convexHullShapePoints;

		for(auto elem: objectData){
			btVector3 co = btVector3(elem.positionX, elem.positionY, elem.positionZ);
			auto itr = std::find(convexHullShapePoints.begin(), convexHullShapePoints.end(), co);
			if( itr == convexHullShapePoints.end() ){
				glm::vec4 target = glm::scale(glm::mat4(1.0f), size) * glm::vec4(co.x(), co.y(), co.z(), 1);
				convexHullShapePoints.push_back(
							btVector3(target.x, target.y, target.z)
						);
			}
		}

		btCollisionShape* shape = new btConvexHullShape( &convexHullShapePoints[0][0], convexHullShapePoints.size(), sizeof(btVector3));

		btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(quat.x, quat.y, quat.z, quat.w), btVector3(position.x, position.y, position.z)));
		btVector3 inertia(0, 0, 0);
		shape->calculateLocalInertia(mass, inertia);
		btRigidBody::btRigidBodyConstructionInfo bodyCI(mass, motionState, shape, inertia);
		btRigidBody* body = new btRigidBody(bodyCI);
		dynamicsWorld->addRigidBody(body);

		objects.push_back(new shapePointerObject(this, true, body, position, size, quat));


		return objects.back();


}

void commonshapeObject::destroy(int id){
}

void commonshapeObject::render(){

	glm::mat4 instanceMatrixArray[objects.size()];

	for(int i = 0; i < objects.size(); i++){
		instanceMatrixArray[i] = objects[i]->loadMatrix();
	}

	glBindBuffer(GL_ARRAY_BUFFER, instanceMatrixBuffer);
	glBufferData(GL_ARRAY_BUFFER, objects.size() * sizeof(glm::mat4), instanceMatrixArray, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, instanceMatrixBuffer);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(sizeof(glm::vec4)*0));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(sizeof(glm::vec4)*1));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(sizeof(glm::vec4)*2));
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(sizeof(glm::vec4)*3));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);

	glDrawElementsInstanced(GL_TRIANGLES, objectData.size(), GL_UNSIGNED_INT, (void*)0, objects.size());

}

shapePointerObject::shapePointerObject(){
}
shapePointerObject::shapePointerObject(commonshapeObject* parent, bool isPhysical, btRigidBody* body, glm::vec3 posi, glm::vec3 size, glm::quat quat){
	this->parent = parent;
	isPhysicalBody = isPhysical;
	initialPosition = posi;
	initialSize = size;
	initialQuat = quat;
	if(isPhysical == true){
		this->body = body;
	}
}

glm::mat4 shapePointerObject::loadMatrix(){
	if(isPhysicalBody == true){
		btTransform transform;
		body->getMotionState()->getWorldTransform(transform);

		btVector3 pos = transform.getOrigin();
		btQuaternion quaternion = transform.getRotation();

		return glm::translate(glm::mat4(1.0f), glm::vec3(pos.getX(), pos.getY(), pos.getZ())) 
			* glm::toMat4(glm::quat(quaternion.getW(), quaternion.getX(), quaternion.getY(), quaternion.getZ()))
			* glm::scale(glm::mat4(1.0f), initialSize);

	}else{
		return glm::translate(glm::mat4(1.0f), initialPosition) 
						* glm::toMat4(initialQuat)
						* glm::scale(glm::mat4(1.0f), initialSize);
	}
}




