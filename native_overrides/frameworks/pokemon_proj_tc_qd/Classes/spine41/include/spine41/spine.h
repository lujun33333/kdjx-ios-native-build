/******************************************************************************
 * Spine Runtimes License Agreement
 * Last updated July 28, 2023. Replaces all prior versions.
 *
 * Copyright (c) 2013-2023, Esoteric Software LLC
 *
 * Integration of the Spine Runtimes into software or otherwise creating
 * derivative works of the Spine Runtimes is permitted under the terms and
 * conditions of Section 2 of the Spine Editor License Agreement:
 * http://esotericsoftware.com/spine-editor-license
 *
 * Otherwise, it is permitted to integrate the Spine Runtimes into software or
 * otherwise create derivative works of the Spine Runtimes (collectively,
 * "Products"), provided that each user of the Products must obtain their own
 * Spine Editor license and redistribution of the Products in any form must
 * include this license and copyright notice.
 *
 * THE SPINE RUNTIMES ARE PROVIDED BY ESOTERIC SOFTWARE LLC "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ESOTERIC SOFTWARE LLC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES,
 * BUSINESS INTERRUPTION, OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THE
 * SPINE RUNTIMES, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef SPINE41_SPINE_H_
#define SPINE41_SPINE_H_

#include <spine41/Animation.h>
#include <spine41/AnimationState.h>
#include <spine41/AnimationStateData.h>
#include <spine41/Atlas.h>
#include <spine41/AtlasAttachmentLoader.h>
#include <spine41/Attachment.h>
#include <spine41/AttachmentLoader.h>
#include <spine41/AttachmentTimeline.h>
#include <spine41/AttachmentType.h>
#include <spine41/BlendMode.h>
#include <spine41/Bone.h>
#include <spine41/BoneData.h>
#include <spine41/BoundingBoxAttachment.h>
#include <spine41/ClippingAttachment.h>
#include <spine41/Color.h>
#include <spine41/ColorTimeline.h>
#include <spine41/ConstraintData.h>
#include <spine41/ContainerUtil.h>
#include <spine41/CurveTimeline.h>
#include <spine41/DeformTimeline.h>
#include <spine41/DrawOrderTimeline.h>
#include <spine41/Event.h>
#include <spine41/EventData.h>
#include <spine41/EventTimeline.h>
#include <spine41/Extension.h>
#include <spine41/HashMap.h>
#include <spine41/HasRendererObject.h>
#include <spine41/IkConstraint.h>
#include <spine41/IkConstraintData.h>
#include <spine41/IkConstraintTimeline.h>
#include <spine41/Json.h>
#include <spine41/LinkedMesh.h>
#include <spine41/MathUtil.h>
#include <spine41/MeshAttachment.h>
#include <spine41/MixBlend.h>
#include <spine41/MixDirection.h>
#include <spine41/PathAttachment.h>
#include <spine41/PathConstraint.h>
#include <spine41/PathConstraintData.h>
#include <spine41/PathConstraintMixTimeline.h>
#include <spine41/PathConstraintPositionTimeline.h>
#include <spine41/PathConstraintSpacingTimeline.h>
#include <spine41/PointAttachment.h>
#include <spine41/Pool.h>
#include <spine41/PositionMode.h>
#include <spine41/RegionAttachment.h>
#include <spine41/RotateMode.h>
#include <spine41/RotateTimeline.h>
#include <spine41/RTTI.h>
#include <spine41/ScaleTimeline.h>
#include <spine41/ShearTimeline.h>
#include <spine41/Skeleton.h>
#include <spine41/SkeletonBinary.h>
#include <spine41/SkeletonBounds.h>
#include <spine41/SkeletonClipping.h>
#include <spine41/SkeletonData.h>
#include <spine41/SkeletonJson.h>
#include <spine41/Skin.h>
#include <spine41/Slot.h>
#include <spine41/SlotData.h>
#include <spine41/SpacingMode.h>
#include <spine41/SpineObject.h>
#include <spine41/SpineString.h>
#include <spine41/TextureLoader.h>
#include <spine41/Timeline.h>
#include <spine41/Property.h>
#include <spine41/TransformConstraint.h>
#include <spine41/TransformConstraintData.h>
#include <spine41/TransformConstraintTimeline.h>
#include <spine41/TransformMode.h>
#include <spine41/TranslateTimeline.h>
#include <spine41/Triangulator.h>
#include <spine41/Updatable.h>
#include <spine41/Vector.h>
#include <spine41/VertexAttachment.h>
#include <spine41/Vertices.h>

#endif
